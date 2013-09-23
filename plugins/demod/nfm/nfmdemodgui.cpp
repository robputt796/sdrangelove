#include <QDockWidget>
#include <QMainWindow>
#include "nfmdemodgui.h"
#include "ui_nfmdemodgui.h"
#include "nfmdemodgui.h"
#include "ui_nfmdemodgui.h"
#include "dsp/threadedsamplesink.h"
#include "dsp/channelizer.h"
#include "nfmdemod.h"
#include "dsp/spectrumvis.h"
#include "gui/glspectrum.h"
#include "plugin/pluginapi.h"
#include "util/simpleserializer.h"

const int NFMDemodGUI::m_rfBW[] = {
	5000, 6250, 8330, 10000, 12500, 15000, 20000, 25000, 40000
};

NFMDemodGUI* NFMDemodGUI::create(PluginAPI* pluginAPI)
{
	QDockWidget* dock = pluginAPI->createMainWindowDock(Qt::RightDockWidgetArea, tr("NFM Demodulator"));
	dock->setObjectName(QString::fromUtf8("NFM Demodulator"));
	NFMDemodGUI* gui = new NFMDemodGUI(pluginAPI, dock);
	dock->setWidget(gui);
	return gui;
}

void NFMDemodGUI::destroy()
{
	delete m_dockWidget;
}

void NFMDemodGUI::setWidgetName(const QString& name)
{
	qDebug("NFM: %s", qPrintable(name));
	m_dockWidget->setObjectName(name);
}

void NFMDemodGUI::resetToDefaults()
{
	ui->rfBW->setValue(4);
	ui->afBW->setValue(3);
	ui->volume->setValue(20);
	ui->squelch->setValue(-40);
	applySettings();
}

QByteArray NFMDemodGUI::serialize() const
{
	SimpleSerializer s(1);
	s.writeS32(1, m_channelMarker->getCenterFrequency());
	s.writeS32(2, ui->rfBW->value());
	s.writeS32(3, ui->afBW->value());
	s.writeS32(4, ui->volume->value());
	s.writeS32(5, ui->squelch->value());
	return s.final();
}

bool NFMDemodGUI::deserialize(const QByteArray& data)
{
	SimpleDeserializer d(data);

	if(!d.isValid()) {
		resetToDefaults();
		return false;
	}

	if(d.getVersion() == 1) {
		qint32 tmp;
		d.readS32(1, &tmp, 0);
		m_channelMarker->setCenterFrequency(tmp);
		d.readS32(2, &tmp, 4);
		ui->rfBW->setValue(tmp);
		d.readS32(3, &tmp, 3);
		ui->afBW->setValue(tmp);
		d.readS32(4, &tmp, 20);
		ui->volume->setValue(tmp);
		d.readS32(5, &tmp, -40);
		ui->squelch->setValue(tmp);
		applySettings();
		return true;
	} else {
		resetToDefaults();
		return false;
	}
}

bool NFMDemodGUI::handleMessage(Message* message)
{
	return false;
}

void NFMDemodGUI::viewChanged()
{
	applySettings();
}

void NFMDemodGUI::on_rfBW_valueChanged(int value)
{
	ui->rfBWText->setText(QString("%1 kHz").arg(m_rfBW[value] / 1000.0));
	m_channelMarker->setBandwidth(m_rfBW[value]);
	applySettings();
}

void NFMDemodGUI::on_afBW_valueChanged(int value)
{
	ui->afBWText->setText(QString("%1 kHz").arg(value));
	applySettings();
}

void NFMDemodGUI::on_volume_valueChanged(int value)
{
	ui->volumeText->setText(QString("%1").arg(value / 10.0, 0, 'f', 1));
	applySettings();
}

void NFMDemodGUI::on_squelch_valueChanged(int value)
{
	ui->squelchText->setText(QString("%1 dB").arg(value));
	applySettings();
}

NFMDemodGUI::NFMDemodGUI(PluginAPI* pluginAPI, QDockWidget* dockWidget, QWidget* parent) :
	PluginGUI(parent),
	ui(new Ui::NFMDemodGUI),
	m_pluginAPI(pluginAPI),
	m_dockWidget(dockWidget)
{
	ui->setupUi(this);

	m_audioFifo = new AudioFifo(4, 44100 / 4);
	m_spectrumVis = new SpectrumVis(ui->glSpectrum);
	m_nfmDemod = new NFMDemod(m_audioFifo, m_spectrumVis);
	m_channelizer = new Channelizer(m_nfmDemod);
	m_threadedSampleSink = new ThreadedSampleSink(m_channelizer);
	m_pluginAPI->addAudioSource(m_audioFifo);
	m_pluginAPI->addSampleSink(m_threadedSampleSink);

	ui->glSpectrum->setCenterFrequency(0);
	ui->glSpectrum->setSampleRate(44100);
	ui->glSpectrum->setDisplayWaterfall(true);
	ui->glSpectrum->setDisplayMaxHold(true);
	m_spectrumVis->configure(m_threadedSampleSink->getMessageQueue(), 64, 10, FFTWindow::BlackmanHarris);

	m_channelMarker = new ChannelMarker(this);
	m_channelMarker->setColor(Qt::red);
	m_channelMarker->setBandwidth(12500);
	m_channelMarker->setCenterFrequency(0);
	m_channelMarker->setVisible(true);
	connect(m_channelMarker, SIGNAL(changed()), this, SLOT(viewChanged()));
	m_pluginAPI->addChannelMarker(m_channelMarker);

	applySettings();
}

NFMDemodGUI::~NFMDemodGUI()
{
	m_pluginAPI->removeAudioSource(m_audioFifo);
	m_pluginAPI->removeSampleSink(m_threadedSampleSink);
	delete m_threadedSampleSink;
	delete m_channelizer;
	delete m_nfmDemod;
	delete m_spectrumVis;
	delete m_audioFifo;
	delete m_channelMarker;
	delete ui;
}

void NFMDemodGUI::applySettings()
{
	m_channelizer->configure(m_threadedSampleSink->getMessageQueue(),
		44100,
		m_channelMarker->getCenterFrequency());
	m_nfmDemod->configure(m_threadedSampleSink->getMessageQueue(),
		m_rfBW[ui->rfBW->value()],
		ui->afBW->value() * 1000.0,
		ui->volume->value() / 10.0,
		ui->squelch->value());
}
