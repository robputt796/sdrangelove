///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 maintech GmbH, Otto-Hahn-Str. 15, 97204 Hoechberg, Germany //
// written by Christian Daniel                                                   //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_GLSPECTRUM_H
#define INCLUDE_GLSPECTRUM_H

#include <QGLWidget>
#include <QTimer>
#include <QMutex>
#include "dsp/dsptypes.h"
#include "scaleengine.h"

class GLSpectrum : public QGLWidget {
	Q_OBJECT

public:
	GLSpectrum(QWidget* parent = NULL);
	~GLSpectrum();

	void setCenterFrequency(quint64 frequency);
	void setSampleRate(qint32 sampleRate);
	void setDisplayWaterfall(bool display);
	void setInvertedWaterfall(bool inv);
	void setDisplayLiveSpectrum(bool display);
	void setDisplayHistogram(bool display);

	void newSpectrum(const std::vector<Real>& spectrum, int fftSize);

private:
	enum CursorState {
		CSNormal,
		CSSplitter,
		CSSplitterMoving
	};

	CursorState m_cursorState;

	QTimer m_timer;
	QMutex m_mutex;
	bool m_changesPending;

	quint64 m_centerFrequency;
	quint32 m_sampleRate;

	int m_fftSize;

	bool m_invertedWaterfall;

	std::vector<Real> m_liveSpectrum;
	bool m_displayLiveSpectrum;
	bool m_liveSpectrumChanged;

	Real m_waterfallShare;

	QPixmap m_leftMarginPixmap;
	bool m_leftMarginTextureAllocated;
	GLuint m_leftMarginTexture;
	QPixmap m_frequencyPixmap;
	bool m_frequencyTextureAllocated;
	GLuint m_frequencyTexture;
	ScaleEngine m_timeScale;
	ScaleEngine m_powerScale;
	ScaleEngine m_frequencyScale;
	QRectF m_glLeftScaleRect;
	QRectF m_glFrequencyScaleRect;
	QRect m_frequencyScaleRect;

	QRgb m_waterfallPalette[240];
	QImage* m_waterfallBuffer;
	int m_waterfallBufferPos;
	bool m_waterfallTextureAllocated;
	GLuint m_waterfallTexture;
	int m_waterfallTextureHeight;
	int m_waterfallTexturePos;
	QRectF m_glWaterfallRect;
	bool m_displayWaterfall;

	QRgb m_histogramPalette[240];
	QImage* m_histogramBuffer;
	quint8* m_histogram;
	quint8* m_histogramHoldoff;
	bool m_histogramTextureAllocated;
	GLuint m_histogramTexture;
	int m_histogramHoldoffBase;
	int m_histogramHoldoffCount;
	int m_histogramLateHoldoff;
	QRectF m_glHistogramRect;
	bool m_displayHistogram;

	bool m_displayChanged;

	void updateWaterfall(const std::vector<Real>& spectrum);
	void updateHistogram(const std::vector<Real>& spectrum);

	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();

	void stopSplitterMove();
	void applyChanges();

	void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);

private slots:
	void tick();
};

#endif // INCLUDE_GLSPECTRUM_H
