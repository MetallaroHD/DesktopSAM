#pragma once
#include <qwidget.h>
#include <qimage.h>
#include <QMouseEvent>

enum ImageCanvasMouseMode { None, Drawing, Erasing, SegmentPoint };

class QtImageCanvas : public QWidget
{
	Q_OBJECT

public:
	explicit QtImageCanvas(QWidget* parent = nullptr);

	QImage image;       
	QImage mask;       
	double zoom = 1.0;   
	QPointF pan;         
	QPointF lastPanPoint;
	bool imageLoaded = false;
	QColor overlayColor = QColor(0, 0, 0, 120);
	ImageCanvasMouseMode mode = None;

public slots:
	void loadImage(const QString& path);
	void resetMask();
	QImage getMaskedImage();
	void setMode(ImageCanvasMouseMode m) { mode = m; }

signals:
	void segmentationPointSelected(const QPoint& screenPos);

private:
	void paintEvent(QPaintEvent*) override;
	void mousePressEvent(QMouseEvent* e) override;
	void wheelEvent(QWheelEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent*) override;

	QPoint mapToImage(const QPoint& p);
	void applyBrush(const QPoint& screenPos, int value);
};