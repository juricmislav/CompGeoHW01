// Convexe Hülle
// (c) Georg Umlauf, 2015
// Edited and developed for Computational Geometry Project by Mislav Jurić

#include "glwidget.h"
#include <QtGui>
#include <GL/glu.h>
#include "mainwindow.h"
#include <iostream>

// functions declaration
void drawGraham();
void drawJarvis();
void drawPolygon(std::vector<QPointF>);
boolean isLeftTurn(std::vector<QPointF>);
QPointF findSmallestPoint();
QPointF findSmallestPointAngle(QPointF, QPointF, std::vector<QPointF>);
float calculateDeterminant(QPointF, QPointF, QPointF);

// class variables
boolean grahamSelected = false;
boolean jarvisSelected = false;
std::vector<QPointF> points = {};

bool compareXCoordinate(QPointF a, QPointF b)
{
	if (a.x() == b.x()) return a.y() < b.y();
	return a.x() < b.x();
}

bool compareYCoordinateSmallest(QPointF a, QPointF b)
{
	if (a.y() == b.y()) return a.x() < b.x();
	return a.y() < b.y();
}

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent)
{
}

GLWidget::~GLWidget()
{
}

void GLWidget::paintGL()
{
	// clear
	glClear(GL_COLOR_BUFFER_BIT);

	// Koordinatensystem
	glColor3f(0.5, 0.5, 0.5);
	glBegin(GL_LINES);
	glVertex2f(-1.0, 0.0);
	glVertex2f(1.0, 0.0);
	glVertex2f(0.0, -1.0);
	glVertex2f(0.0, 1.0);
	glEnd();

	if (points.size() > 0)
	{
		glColor3f(1, 0, 0);
		glBegin(GL_POINTS);
		for (auto &point : points)
		{
			glVertex2f(point.x(), point.y());
		}
		glEnd();
	}

	if (grahamSelected) drawGraham();
	if (jarvisSelected) drawJarvis();

}


void GLWidget::initializeGL()
{
	resizeGL(width(), height());
}

void GLWidget::resizeGL(int width, int height)
{
	aspectx = 1.0;
	aspecty = 1.0;
	if (width>height) aspectx = float(width) / height;
	else              aspecty = float(height) / width;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-aspectx, aspectx, -aspecty, aspecty);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

QPointF GLWidget::transformPosition(QPoint p)
{
	return QPointF((2.0*p.x() / width() - 1.0)*aspectx,
		-(2.0*p.y() / height() - 1.0)*aspecty);
}

void GLWidget::keyPressEvent(QKeyEvent * event)
{
	switch (event->key()) {
	default:
		break;
	}
	update();
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
	QPointF posF = transformPosition(event->pos());
	if (event->buttons() & Qt::LeftButton) {
		points.push_back(posF);
	}
	update();
}


void GLWidget::radioButton1Clicked()
{
	// Jarvis' March selected
	grahamSelected = false;
	jarvisSelected = true;
	update();
}

void GLWidget::radioButton2Clicked()
{
	// Graham's Scan selected
	grahamSelected = true;
	jarvisSelected = false;
	update();
}

void drawGraham()
{
	std::sort(points.begin(), points.end(), compareXCoordinate);
	
	std::vector<QPointF> upEdges = {};
	std::vector<QPointF> lowEdges = {};

	if (points.size() >= 3)
	{
		int pointSize = points.size();
		upEdges.push_back(points.at(0));
		upEdges.push_back(points.at(1));
		for (int i = 2; i < pointSize; ++i)
		{
			upEdges.push_back(points.at(i));
			while (upEdges.size() > 2 && isLeftTurn(upEdges))
			{
				upEdges.erase(upEdges.end() - 2);
			}
		}

		lowEdges.push_back(points.at(pointSize - 1));
		lowEdges.push_back(points.at(pointSize - 2));
		for (int i = pointSize - 3; i >= 0; --i)
		{
			lowEdges.push_back(points.at(i));
			while (lowEdges.size() > 2 && isLeftTurn(lowEdges))
			{
				lowEdges.erase(lowEdges.end() - 2);
			}
		}

		if (lowEdges.size() >= 2)
		{
			lowEdges.erase(lowEdges.end() - 1);
			lowEdges.erase(lowEdges.begin());
		}
		upEdges.insert(std::end(upEdges), std::begin(lowEdges), std::end(lowEdges));
	}

	drawPolygon(upEdges);
}

boolean isLeftTurn(std::vector<QPointF> points)
{
	int size = points.size();

	QPointF a = points.at(size - 3);
	QPointF b = points.at(size - 2);
	QPointF c = points.at(size - 1);

	return calculateDeterminant(a, b, c) > 0 ? true : false;
}

float calculateDeterminant(QPointF a, QPointF b, QPointF c)
{
	float ABx = b.x() - a.x();
	float ABy = b.y() - a.y();
	float ACx = c.x() - a.x();
	float ACy = c.y() - a.y();
	
	return ABx * ACy - ABy * ACx;
}

float calculateAngleDotProduct(QPointF a, QPointF b, QPointF c)
{
	float ABx = b.x() - a.x();
	float ABy = b.y() - a.y();
	float CBx = b.x() - c.x();
	float CBy = b.y() - c.y();

	// calculating dot product: [ABx, ABy] and [CBx, CBy]
	float dot = ABx * CBx + ABy * CBy;
	// calculating product of modules: |AB|*|BC|
	float module = sqrt(ABx * ABx + ABy * ABy) * sqrt(CBx * CBx + CBy * CBy);
	// calculating angle
	float cos = dot / module;

	return 2 * M_PI - acos(cos);
}

void drawJarvis()
{
	if (points.size() < 3) return;
	std::vector<QPointF> sortedPoints = {};
	for (QPointF &point : points)
	{
		sortedPoints.push_back(point);
	}
	std::sort(sortedPoints.begin(), sortedPoints.end(), compareYCoordinateSmallest);

	QPointF p = sortedPoints.at(0);
	std::vector<QPointF> edges = {};
	edges.push_back(p);

	QPointF q = findSmallestPointAngle(p, QPointF(qreal(p.x() + 0.0001), qreal(p.y())), sortedPoints);
	edges.push_back(q);
	sortedPoints.erase(std::remove(sortedPoints.begin(), sortedPoints.end(), q), sortedPoints.end());

	while (q != p)
	{
		q = findSmallestPointAngle(edges.at(edges.size() - 2), edges.at(edges.size() - 1), sortedPoints);
		edges.push_back(q);
	}

	if (edges.size() >= 3) drawPolygon(edges);
}

QPointF findSmallestPointAngle(QPointF startPoint, QPointF endPoint, std::vector<QPointF> sortedPoints)
{
	QPointF closestPoint;
	float smallestAngle = 2 * M_PI;
	for (QPointF &point : sortedPoints)
	{
		float angle = calculateAngleDotProduct(startPoint, endPoint, point);
		if (angle < smallestAngle)
		{
			smallestAngle = angle;
			closestPoint = point;
		}
	}
	sortedPoints.erase(std::remove(sortedPoints.begin(), sortedPoints.end(), closestPoint), sortedPoints.end());
	return closestPoint;
}

void drawPolygon(std::vector<QPointF> edges)
{
	if (edges.size() >= 3)
	{
		glColor3f(1, 1, 0);
		glBegin(GL_LINE_LOOP);
		for (auto &point : edges)
		{
			glVertex2f(point.x(), point.y());
		}
		glEnd();
	}
}
