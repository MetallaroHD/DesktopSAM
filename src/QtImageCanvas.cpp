#include "QtImageCanvas.h"
#include <QPainter>
#include <QImageReader>

QtImageCanvas::QtImageCanvas(QWidget* parent)
{
}

void QtImageCanvas::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::black);

    if (!imageLoaded)
        return;

    p.translate(pan);
    p.scale(zoom, zoom);

    p.drawImage(QPoint(0, 0), image);

    if (!mask.isNull())
    {
        bool anyForeground = false;

        // Check if the mask has any non-zero pixel
        for (int y = 0; y < image.height() && !anyForeground; ++y)
        {
            const uchar* mLine = mask.constScanLine(y);
            for (int x = 0; x < image.width(); ++x)
            {
                if (mLine[x] > 0)
                {
                    anyForeground = true;
                    break;
                }
            }
        }

        if (anyForeground)
        {
            QImage overlay(image.size(), QImage::Format_ARGB32);
            overlay.fill(Qt::transparent);

            for (int y = 0; y < image.height(); ++y)
            {
                const uchar* mLine = mask.constScanLine(y);
                QRgb* outLine = reinterpret_cast<QRgb*>(overlay.scanLine(y));

                for (int x = 0; x < image.width(); ++x)
                {
                    if (mLine[x] == 0)
                        outLine[x] = qRgba(overlayColor.red(), overlayColor.green(), overlayColor.blue(), overlayColor.alpha());
                }
            }

            p.drawImage(QPoint(0, 0), overlay);
        }
    }
}

void QtImageCanvas::mousePressEvent(QMouseEvent* e)
{
    if (!imageLoaded) return;

    if (mode == SegmentPoint)
    {
        if (e->button() == Qt::LeftButton)
        {
            QPoint imgPt = mapToImage(e->pos());
            emit segmentationPointSelected(imgPt);
            mode = None;
            setCursor(Qt::ArrowCursor);
        }
        if (e->button() == Qt::RightButton)
        {
            mode = None;
            setCursor(Qt::ArrowCursor);
        }    
    }
    else
    {
        if (e->button() == Qt::LeftButton)
            mode = Drawing;
        else if (e->button() == Qt::RightButton)
            mode = Erasing;
        if (e->button() == Qt::MiddleButton)
            lastPanPoint = e->pos();
    }
}

void QtImageCanvas::wheelEvent(QWheelEvent* event)
{
    if (!imageLoaded)
        return;

    double delta = event->angleDelta().y() / 120.0;
    double factor = std::pow(1.15, delta);
    double newZoom = zoom * factor;

    if (newZoom < 0.1) newZoom = 0.1;
    if (newZoom > 10.0) newZoom = 10.0;

    QPointF cursorPos = event->position();
    QPointF imgPosBefore = (cursorPos - pan) / zoom;

    zoom = newZoom;
    pan = cursorPos - imgPosBefore * zoom;

    update();
}

void QtImageCanvas::mouseMoveEvent(QMouseEvent* e)
{
    if (!imageLoaded)
        return;

    if (mode == Drawing)
        applyBrush(e->pos(), +1);

    else if (mode == Erasing)
        applyBrush(e->pos(), 0);

    else if (e->buttons() & Qt::MiddleButton)
    {
        QPointF delta = e->pos() - lastPanPoint;
        pan += delta;
        lastPanPoint = e->pos();
        update();
    }
}

void QtImageCanvas::mouseReleaseEvent(QMouseEvent*)
{
    mode = None;
}

QPoint QtImageCanvas::mapToImage(const QPoint& p)
{
    QPointF pt = (p - pan) / zoom;
    return pt.toPoint();
}

void QtImageCanvas::applyBrush(const QPoint& screenPos, int value)
{
    QPointF ipF = (screenPos - pan) / zoom;
    int cx = int(std::floor(ipF.x()));
    int cy = int(std::floor(ipF.y()));

    if (!mask.rect().contains(cx, cy))
        return;

    uchar v = value ? 255 : 0;

    uchar* base = mask.bits();
    int stride = mask.bytesPerLine();

    double raw = 10 / zoom;
    int size = int(std::ceil(raw));

    if (size < 1) size = 1;
    if (size % 2 == 0) size += 1; 

    int r = size / 2;

    for (int dy = -r; dy <= r; ++dy)
    {
        for (int dx = -r; dx <= r; ++dx)
        {
            int x = cx + dx;
            int y = cy + dy;

            if (mask.rect().contains(x, y))
                base[y * stride + x] = v;
        }
    }

    update();
}

void QtImageCanvas::resetMask()
{
    if (!imageLoaded)
        return;

    mask.fill(0);
    update();
}

QImage QtImageCanvas::getMaskedImage()
{
    if (!imageLoaded)
        return QImage();

    QImage output(image.size(), QImage::Format_ARGB32);
    output.fill(Qt::transparent);

    for (int y = 0; y < image.height(); ++y)
    {
        const uchar* maskLine = mask.constScanLine(y);
        const QRgb* imgLine = reinterpret_cast<const QRgb*>(image.constScanLine(y));
        QRgb* outLine = reinterpret_cast<QRgb*>(output.scanLine(y));

        for (int x = 0; x < image.width(); ++x)
        {
            if (maskLine[x] > 0)
            {
                outLine[x] = qRgba(
                    qRed(imgLine[x]),
                    qGreen(imgLine[x]),
                    qBlue(imgLine[x]),
                    255
                );
            }
            else
            {
                outLine[x] = qRgba(0, 0, 0, 0);
            }
        }
    }

    return output;
}

void QtImageCanvas::loadImage(const QString& path)
{
    QImageReader reader(path);
    reader.setAutoTransform(true);

    QImage img = reader.read();
    if (img.isNull())
    {
        qWarning() << "Failed to load image:" << path << reader.errorString();
        return;
    }

    image = img;
    imageLoaded = true;

    mask = QImage(image.size(), QImage::Format_Grayscale8);
    mask.fill(0);

    zoom = 1.0;
    pan = QPointF(0, 0);

    update();
}
