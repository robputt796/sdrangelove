#ifndef INCLUDE_ROLLUPWIDGET_H
#define INCLUDE_ROLLUPWIDGET_H

#include <QWidget>
#include "util/export.h"

class SDRANGELOVE_API RollupWidget : public QWidget {
	Q_OBJECT

public:
	RollupWidget(QWidget* parent = NULL);

	QByteArray saveState(int version = 0) const;
	bool restoreState(const QByteArray& state, int version = 0);

signals:
	void widgetRolled(QWidget* widget, bool rollDown);
	void menuDoubleClickEvent(const QPoint& position);

protected:
	enum {
		VersionMarker = 0xff
	};

	int arrangeRollups();

	void paintEvent(QPaintEvent*);
	int paintRollup(QWidget* rollup, int pos, QPainter* p, bool last, const QColor& frame);

	void resizeEvent(QResizeEvent* size);
	void mousePressEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);

	bool event(QEvent* event);
	bool eventFilter(QObject* object, QEvent* event);
};

#endif // INCLUDE_ROLLUPWIDGET_H
