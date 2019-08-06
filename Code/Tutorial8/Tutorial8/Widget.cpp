#include "Widget.h"

/*
Description:
	This function is a constructor;
Input:
	@ QWidget* parent:
*/
Widget::Widget(QWidget* parent) :
	QOpenGLWidget(parent) {
	camera = new Camera3D;
	camera->translate(QVector3D(0.0, 0.0, -5.0));
}

/*
Description:
	This function is a destructor;
Input:
	@ void patameter: void;
*/
Widget::~Widget() {
	delete camera;

	for (int i = 0; i < objects.size(); i++)
		delete objects[i];

	for (int i = 0; i < groups.size(); i++)
		delete groups[i];

	for (int i = 0; i < transformObjects.size(); i++)
		delete transformObjects[i];
}

/*
Description:
	This function is used to initialize OpenGL state machine, and initialize shaders ,objects and etc.;
Input:
	@ void parameter: void;
Output:
	@ void returnValue: void;
*/
void Widget::initializeGL() {

	setFocusPolicy(Qt::StrongFocus);

	// clear the screen with black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// initialize shaders
	initShaders();

	float step = 1.0f;

	groups.append(new Group3D);
	for (float x = -step; x <= step; x += 2 * step) {
		for (float y = -step; y <= step; y += 2 * step) {
			for (float z = -step; z <= step; z += 2 * step) {
				initCube(0.5f);
				objects[objects.size() - 1]->translate(QVector3D(x, y, z));
				groups[groups.size() - 1]->addObject(objects[objects.size() - 1]);
			}
		}
	}
	groups[0]->translate(QVector3D(-4.0, 0.0, 0.0));

	groups.append(new Group3D);
	for (float x = -step; x <= step; x += 2 * step) {
		for (float y = -step; y <= step; y += 2 * step) {
			for (float z = -step; z <= step; z += 2 * step) {
				initCube(0.5f);
				objects[objects.size() - 1]->translate(QVector3D(x, y, z));
				groups[groups.size() - 1]->addObject(objects[objects.size() - 1]);
			}
		}
	}
	groups[1]->translate(QVector3D(4.0, 0.0, 0.0));

	groups.append(new Group3D);
	groups[2]->addObject(groups[0]);
	groups[2]->addObject(groups[1]);

	transformObjects.append(groups[2]);

	loadObj("./sphere.obj");
	transformObjects.append(objects[objects.size() - 1]);

	groups[0]->addObject(camera);

	skybox = new Skybox(40, QImage("./skybox.jpg").mirrored());

	 timer.start(10, this);
}

/*
Description:
	This function is used to deal with resive event;
Input:
	@ int width: window width after resize event;
	@ int height: window height after resize event;
Output:
	@ void returnValue: void;
*/
void Widget::resizeGL(int width, int height) {
	float aspect = width / (float)height;

	pMatrix.setToIdentity();
	pMatrix.perspective(45, aspect, 0.01f, 500.0f);
}

/*
Description:
	This function is used to set parameters for the vertex shader, fragment shader and etc. and draw other objects;
Input:
	@ void parameter: void;
Output:
	@ void returnValue: void;
*/
void Widget::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	objectShader.bind();
	objectShader.setUniformValue("u_projectionMatrix", pMatrix);
	objectShader.setUniformValue("u_lightPosition", QVector4D(0.0, 0.0, 0.0, 1.0));
	objectShader.setUniformValue("u_lightPower", 3.0f);

	camera->draw(&objectShader);
	for (int i = 0; i < transformObjects.size(); i++) {
		transformObjects[i]->draw(&objectShader, context()->functions());
	}
	objectShader.release();

	skyboxShader.bind();
	skyboxShader.setUniformValue("u_projectionMatrix", pMatrix);

	camera->draw(&skyboxShader);
	skybox->draw(&skyboxShader, context()->functions());
	skyboxShader.release();
}

/*
Description:
	This function is used to process mouse events, which is a Qt event function;
Intput:
	@ QKeyEvent* event: a mouse event;
Output:
	@ void returnValue: void;
*/
void Widget::mousePressEvent(QMouseEvent* event) {
	if (event->buttons() == Qt::LeftButton)
		mousePosition = QVector2D(event->localPos());
	event->accept();
	update();
}

/*
Description:
	This function is used to process mouse move events, which is a Qt event function;
Intput:
	@ QKeyEvent* event: a mouse move event;
Output:
	@ void returnValue: void;
*/
void Widget::mouseMoveEvent(QMouseEvent* event) {
	if (event->buttons() != Qt::LeftButton) return;

	QVector2D diff = QVector2D(event->localPos()) - mousePosition;
	mousePosition = QVector2D(event->localPos());

	float angle = diff.length();

	QVector3D axis = QVector3D(diff.y(), diff.x(), 0.0);

	camera->rotate(QQuaternion::fromAxisAndAngle(axis, angle));

	update();
}

/*
Description:
	This function is used to process wheel events, which is a Qt event function;
Intput:
	@ QKeyEvent* event: a wheel event;
Output:
	@ void returnValue: void;
*/
void Widget::wheelEvent(QWheelEvent* event) {
	if (event->delta() > 0) {
		camera->translate(QVector3D(0.0, 0.0, 0.25));
	}
	else if (event->delta() < 0) {
		camera->translate(QVector3D(0.0, 0.0, -0.25));
	}
	update();
}

/*
Description:
	This function is used to process timer events, which is a Qt event function;
Intput:
	@ QKeyEvent* event: a timer event;
Output:
	@ void returnValue: void;
*/
void Widget::timerEvent(QTimerEvent* event) {
	for (int i = 0; i < objects.size() - 1; i++) {
		if (i % 2 == 0) {
			objects[i]->rotate(QQuaternion::fromAxisAndAngle(1.0, 0.0, 0.0, qSin(angleObject)));
			objects[i]->rotate(QQuaternion::fromAxisAndAngle(0.0, 1.0, 0.0, qCos(angleObject)));
		}
		else {
			objects[i]->rotate(QQuaternion::fromAxisAndAngle(0.0, 1.0, 0.0, qSin(angleObject)));
			objects[i]->rotate(QQuaternion::fromAxisAndAngle(1.0, 0.0, 0.0, qCos(angleObject)));
		}
	}

	groups[0]->rotate(QQuaternion::fromAxisAndAngle(0.0, 0.0, 1.0, qSin(angleGroup1)));
	groups[0]->rotate(QQuaternion::fromAxisAndAngle(0.0, 1.0, 0.0, -qSin(angleGroup1)));

	groups[1]->rotate(QQuaternion::fromAxisAndAngle(1.0, 0.0, 0.0, qCos(angleGroup1)));
	groups[1]->rotate(QQuaternion::fromAxisAndAngle(0.0, 1.0, 0.0, -qCos(angleGroup1)));

	groups[2]->rotate(QQuaternion::fromAxisAndAngle(1.0, 0.0, 0.0, qSin(angleMain)));
	groups[2]->rotate(QQuaternion::fromAxisAndAngle(0.0, 1.0, 0.0, qCos(angleMain)));

	angleObject += M_PI / 180.0f;
	angleGroup1 += M_PI / 360.0f;
	angleGroup2 -= M_PI / 360.0f;
	angleMain += M_PI / 720.0f;

	update();
}

/*
Description:
	This function is used to process key events, which is a Qt event function;
Intput:
	@ QKeyEvent* event: a key event;
Output:
	@ void returnValue: void;
*/
void Widget::keyPressEvent(QKeyEvent* event) {

	switch (event->key()) {
	case Qt::Key_Left:
		groups[0]->delObject(camera);
		groups[1]->addObject(camera);
		break;
	case Qt::Key_Right:
		groups[1]->delObject(camera);
		groups[0]->addObject(camera);
		break;
	case Qt::Key_Down:
		groups[0]->delObject(camera);
		groups[1]->delObject(camera);
break;
	case Qt::Key_Up:
		groups[0]->delObject(camera);
		groups[1]->delObject(camera);
		QMatrix4x4 temp;
		temp.setToIdentity();
		camera->setGlobalTransform(temp);
		break;
	}
	update();
}

/*
Description:
	This function is used to initialize shaders objects;
Input:
	@ void parameter: void;
Output:
	@ void returnValue: void;
*/
void Widget::initShaders() {
	if (!objectShader.addShaderFromSourceFile(QOpenGLShader::Vertex, "./Object.vsh")) {
		QString log = objectShader.log();
		close();
	}
	if (!objectShader.addShaderFromSourceFile(QOpenGLShader::Fragment, "./Object.fsh")) {
		QString log = objectShader.log();
		close();
	}
	if (!objectShader.link()) {
		QString log = objectShader.log();
		close();
	}

	if (!skyboxShader.addShaderFromSourceFile(QOpenGLShader::Vertex, "./Skybox.vsh")) {
		QString log = skyboxShader.log();
		close();
	}
	if (!skyboxShader.addShaderFromSourceFile(QOpenGLShader::Fragment, "./Skybox.fsh")) {
		QString log = skyboxShader.log();
		close();
	}
	if (!skyboxShader.link()) {
		QString log = skyboxShader.log();
		close();
	}
}

/*
Description:
	This function is used to load graphics data for a cube, including vertex data and index data;
Input:
	@ int width: the width of the cube;
Output:
	@ void returnValue: void;
*/
void Widget::initCube(float width) {
	QVector<Vertex> vertices;
	vertices <<
		Vertex(QVector3D(-width, width, width), QVector2D(0.0, 1.0), QVector3D(0.0, 0.0, 1.0)) <<
		Vertex(QVector3D(-width, -width, width), QVector2D(0.0, 0.0), QVector3D(0.0, 0.0, 1.0)) <<
		Vertex(QVector3D(width, width, width), QVector2D(1.0, 1.0), QVector3D(0.0, 0.0, 1.0)) <<
		Vertex(QVector3D(width, -width, width), QVector2D(1.0, 0.0), QVector3D(0.0, 0.0, 1.0)) <<

		Vertex(QVector3D(width, width, width), QVector2D(0.0, 1.0), QVector3D(1.0, 0.0, 0.0)) <<
		Vertex(QVector3D(width, -width, width), QVector2D(0.0, 0.0), QVector3D(1.0, 0.0, 0.0)) <<
		Vertex(QVector3D(width, width, -width), QVector2D(1.0, 1.0), QVector3D(1.0, 0.0, 0.0)) <<
		Vertex(QVector3D(width, -width, -width), QVector2D(1.0, 0.0), QVector3D(1.0, 0.0, 0.0)) <<

		Vertex(QVector3D(width, width, width), QVector2D(0.0, 1.0), QVector3D(0.0, 1.0, 0.0)) <<
		Vertex(QVector3D(width, width, -width), QVector2D(0.0, 0.0), QVector3D(0.0, 1.0, 0.0)) <<
		Vertex(QVector3D(-width, width, width), QVector2D(1.0, 1.0), QVector3D(0.0, 1.0, 0.0)) <<
		Vertex(QVector3D(-width, width, -width), QVector2D(1.0, 0.0), QVector3D(0.0, 1.0, 0.0)) <<

		Vertex(QVector3D(width, width, -width), QVector2D(0.0, 1.0), QVector3D(0.0, 0.0, -1.0)) <<
		Vertex(QVector3D(width, -width, -width), QVector2D(0.0, 0.0), QVector3D(0.0, 0.0, -1.0)) <<
		Vertex(QVector3D(-width, width, -width), QVector2D(1.0, 1.0), QVector3D(0.0, 0.0, -1.0)) <<
		Vertex(QVector3D(-width, -width, -width), QVector2D(1.0, 0.0), QVector3D(0.0, 0.0, -1.0)) <<

		Vertex(QVector3D(-width, width, width), QVector2D(0.0, 1.0), QVector3D(-1.0, 0.0, 0.0)) <<
		Vertex(QVector3D(-width, width, -width), QVector2D(0.0, 0.0), QVector3D(-1.0, 0.0, 0.0)) <<
		Vertex(QVector3D(-width, -width, width), QVector2D(1.0, 1.0), QVector3D(-1.0, 0.0, 0.0)) <<
		Vertex(QVector3D(-width, -width, -width), QVector2D(1.0, 0.0), QVector3D(-1.0, 0.0, 0.0)) <<

		Vertex(QVector3D(-width, -width, width), QVector2D(0.0, 1.0), QVector3D(0.0, -1.0, 0.0)) <<
		Vertex(QVector3D(-width, -width, -width), QVector2D(0.0, 0.0), QVector3D(0.0, -1.0, 0.0)) <<
		Vertex(QVector3D(width, -width, width), QVector2D(1.0, 1.0), QVector3D(0.0, -1.0, 0.0)) <<
		Vertex(QVector3D(width, -width, -width), QVector2D(1.0, 0.0), QVector3D(0.0, -1.0, 0.0));

	QVector<GLuint> indices;
	indices << 0 << 1 << 2 << 2 << 1 << 3;
	for (int i = 0; i < 24; i += 4)
		indices << i + 0 << i + 1 << i + 2 << i + 2 << i + 1 << i + 3;

	objects << new SimpleObject3D(vertices, indices, QImage("./cube.jpg"));
}

/*
Description:
	This function is used to load .obj file from a given filepath, the .obj file should include
	vertex coordinations [v], texture coordinations [vt], normals [vn], vertex indices of a given face [f], material library file name [mtllib], material name [usemtl].
	If the .obj file doesn't have a material library file name [mtllib], there will be a default material generated when Object is contructed.
Input:
	@ const QString & filePath: the path refer to the .obj file
Output:
	@ void returnValue: void;
*/
void Widget::loadObj(const QString& filePath) {
	QFile objFile(filePath);
	if (!objFile.exists()) {
		return;
	}

	QVector<QVector3D> verCoords;
	QVector<QVector2D> texCoords;
	QVector<QVector3D> normals;
	QVector<Vertex> vertices;
	QVector<GLuint> indices;

	objFile.open(QIODevice::ReadOnly);
	QTextStream input(&objFile);

	while (!input.atEnd()) {
		QString line = input.readLine();
		QStringList list = line.split(" ");
		if (list[0] == "v") {
			verCoords << QVector3D(list[1].toFloat(), list[2].toFloat(), list[3].toFloat());
		}
		else if (list[0] == "vt") {
			texCoords << QVector2D(list[1].toFloat(), list[2].toFloat());
		}
		else if (list[0] == "vn"){
			normals << QVector3D(list[1].toFloat(), list[2].toFloat(), list[3].toFloat());
		}
		else if (list[0] == "f") {
			for (int i = 1; i <= 3; i++) {
				QStringList v = list[i].split("/");
				vertices.append(Vertex(verCoords[v[0].toLong() - 1], texCoords[v[1].toLong() - 1], normals[v[2].toLong() - 1]));
				indices.append(indices.size());
			}
		}
	}

	objFile.close();

	objects.append(new SimpleObject3D(vertices, indices, QImage("./cube.jpg")));
}
