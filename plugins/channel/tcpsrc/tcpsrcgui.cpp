#include "tcpsrcgui.h"
#include "plugin/pluginapi.h"
#include "tcpsrc.h"
#include "dsp/channelizer.h"
#include "dsp/spectrumvis.h"
#include "dsp/threadedsamplesink.h"
#include "ui_tcpsrcgui.h"

TCPSrcGUI* TCPSrcGUI::create(PluginAPI* pluginAPI)
{
	TCPSrcGUI* gui = new TCPSrcGUI(pluginAPI);
	return gui;
}

void TCPSrcGUI::destroy()
{
	delete this;
}

void TCPSrcGUI::setName(const QString& name)
{
	setObjectName(name);
}

void TCPSrcGUI::resetToDefaults()
{
	ui->sampleFormat->setCurrentIndex(0);
	ui->sampleRate->setText("25000");
	ui->rfBandwidth->setText("20000");
	ui->tcpPort->setText("9999");
	applySettings();
}

QByteArray TCPSrcGUI::serialize() const
{
	return QByteArray();
}

bool TCPSrcGUI::deserialize(const QByteArray& data)
{
	return false;
}

bool TCPSrcGUI::handleMessage(Message* message)
{
	if(message->id() == TCPSrc::MsgTCPSrcConnection::ID()) {
		TCPSrc::MsgTCPSrcConnection* con = (TCPSrc::MsgTCPSrcConnection*)message;
		if(con->getConnect())
			addConnection(con->id(), con->getPeerAddress(), con->getPeerPort());
		else delConnection(con->id());
		message->completed();
		return true;
	} else {
		return false;
	}
}

void TCPSrcGUI::channelMarkerChanged()
{
	applySettings();
}

TCPSrcGUI::TCPSrcGUI(PluginAPI* pluginAPI, QWidget* parent) :
	RollupWidget(parent),
	ui(new Ui::TCPSrcGUI),
	m_pluginAPI(pluginAPI)
{
	ui->setupUi(this);
	ui->connectedClientsBox->hide();
	setAttribute(Qt::WA_DeleteOnClose, true);

	m_spectrumVis = new SpectrumVis(ui->glSpectrum);
	m_tcpSrc = new TCPSrc(m_pluginAPI->getMainWindowMessageQueue(), this, m_spectrumVis);
	m_channelizer = new Channelizer(m_tcpSrc);
	m_threadedSampleSink = new ThreadedSampleSink(m_channelizer);
	m_pluginAPI->addSampleSink(m_threadedSampleSink);

	ui->glSpectrum->setCenterFrequency(0);
	ui->glSpectrum->setSampleRate(ui->sampleRate->text().toInt());
	ui->glSpectrum->setDisplayWaterfall(true);
	ui->glSpectrum->setDisplayMaxHold(true);
	m_spectrumVis->configure(m_threadedSampleSink->getMessageQueue(), 64, 10, FFTWindow::BlackmanHarris);

	m_channelMarker = new ChannelMarker(this);
	m_channelMarker->setColor(Qt::red);
	m_channelMarker->setBandwidth(25000);
	m_channelMarker->setCenterFrequency(0);
	m_channelMarker->setVisible(true);
	connect(m_channelMarker, SIGNAL(changed()), this, SLOT(channelMarkerChanged()));
	m_pluginAPI->addChannelMarker(m_channelMarker);

	applySettings();
}

TCPSrcGUI::~TCPSrcGUI()
{
	m_pluginAPI->removeChannelInstance(this);
	m_pluginAPI->removeSampleSink(m_threadedSampleSink);
	delete m_threadedSampleSink;
	delete m_channelizer;
	delete m_tcpSrc;
	delete m_spectrumVis;
	delete m_channelMarker;
	delete ui;
}

void TCPSrcGUI::applySettings()
{
	bool ok;

	Real outputSampleRate = ui->sampleRate->text().toDouble(&ok);
	if((!ok) || (outputSampleRate < 100))
		outputSampleRate = 25000;
	Real rfBandwidth = ui->rfBandwidth->text().toDouble(&ok);
	if((!ok) || (rfBandwidth > outputSampleRate))
		rfBandwidth = outputSampleRate / 1.05;
	int tcpPort = ui->tcpPort->text().toInt(&ok);
	if((!ok) || (tcpPort < 1) || (tcpPort > 65535))
		tcpPort = 9999;

	ui->sampleRate->setText(QString("%1").arg(outputSampleRate, 0));
	ui->rfBandwidth->setText(QString("%1").arg(rfBandwidth, 0));
	ui->tcpPort->setText(QString("%1").arg(tcpPort));
	m_channelMarker->disconnect(this, SLOT(channelMarkerChanged()));
	m_channelMarker->setBandwidth((int)rfBandwidth);
	connect(m_channelMarker, SIGNAL(changed()), this, SLOT(channelMarkerChanged()));
	ui->glSpectrum->setSampleRate(outputSampleRate);

	m_channelizer->configure(m_threadedSampleSink->getMessageQueue(),
		outputSampleRate,
		m_channelMarker->getCenterFrequency());

	TCPSrc::SampleFormat sampleFormat;
	switch(ui->sampleFormat->currentIndex()) {
		case 0:
			sampleFormat = TCPSrc::FormatS8;
			break;
		case 1:
			sampleFormat = TCPSrc::FormatS16LE;
			break;
		default:
			sampleFormat = TCPSrc::FormatS8;
			break;
	}

	m_tcpSrc->configure(m_threadedSampleSink->getMessageQueue(),
		sampleFormat,
		outputSampleRate,
		rfBandwidth,
		tcpPort);

	ui->applyBtn->setEnabled(false);
}

void TCPSrcGUI::on_sampleFormat_currentIndexChanged(int index)
{
	ui->applyBtn->setEnabled(true);
}

void TCPSrcGUI::on_sampleRate_textEdited(const QString& arg1)
{
	ui->applyBtn->setEnabled(true);
}

void TCPSrcGUI::on_rfBandwidth_textEdited(const QString& arg1)
{
	ui->applyBtn->setEnabled(true);
}

void TCPSrcGUI::on_tcpPort_textEdited(const QString& arg1)
{
	ui->applyBtn->setEnabled(true);
}

void TCPSrcGUI::on_applyBtn_clicked()
{
	applySettings();
}

void TCPSrcGUI::addConnection(quint32 id, const QHostAddress& peerAddress, int peerPort)
{
	QStringList l;
	l.append(QString("%1:%2").arg(peerAddress.toString()).arg(peerPort));
	new QTreeWidgetItem(ui->connections, l, id);
	ui->connectedClientsBox->setWindowTitle(tr("Connected Clients (%1)").arg(ui->connections->topLevelItemCount()));
}

void TCPSrcGUI::delConnection(quint32 id)
{
	for(int i = 0; i < ui->connections->topLevelItemCount(); i++) {
		if(ui->connections->topLevelItem(i)->type() == id) {
			delete ui->connections->topLevelItem(i);
			ui->connectedClientsBox->setWindowTitle(tr("Connected Clients (%1)").arg(ui->connections->topLevelItemCount()));
			return;
		}
	}
}