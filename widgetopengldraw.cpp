#include "widgetopengldraw.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <QOpenGLFunctions_3_3_Core>
#include <QtGlobal>
#if QT_VERSION >= 0x060000
#include <QOpenGLVersionFunctionsFactory>
#endif

#include <QApplication>

WidgetOpenGLDraw::WidgetOpenGLDraw(QWidget *parent) : QOpenGLWidget(parent) {
}

WidgetOpenGLDraw::~WidgetOpenGLDraw() {
	//počisti stanje
    gl->glDeleteVertexArrays(1,&id_VAO_trikotnik);
    gl->glDeleteProgram(id_sencilni_program);
}


void WidgetOpenGLDraw::printProgramInfoLog(GLuint obj) {
    int infologLength = 0;
    gl->glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
    if (infologLength > 0) {
        std::unique_ptr<char[]> infoLog(new char[infologLength]);
        int charsWritten = 0;
        gl->glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog.get());
        std::cerr << infoLog.get() << "\n";
    }
}

void WidgetOpenGLDraw::printShaderInfoLog(GLuint obj) {
    int infologLength = 0;
    gl->glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
    if (infologLength > 0) {
		std::unique_ptr<char[]> infoLog(new char[infologLength]);
		int charsWritten = 0;
        gl->glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog.get());
		std::cerr << infoLog.get() << "\n";
	}
}

void WidgetOpenGLDraw::PrevediSencilnike() {
    id_sencilni_program = gl->glCreateProgram();

	{  // vhod v senčilnik oglišč je in_Pos, izhod pa gl_Position (rezervirana beseda)
        GLuint vs = gl->glCreateShader(GL_VERTEX_SHADER);
        std::string vss;
        vss += " #version 330	                                 \n";
        vss += " layout(location=0) in vec3 in_Pos;	             \n";
		vss += " uniform mat4 PVM;						         \n";
		vss += " out vec3 gPos;						             \n";
        vss += " void main(){						             \n";
        vss += "  gl_Position=PVM*vec4(in_Pos.xyz,1);          \n";

        vss+="      gPos=in_Pos;                                 \n";
        vss += " }                                               \n ";
		std::cout << vss;
		const char *vssc = vss.c_str();
        gl->glShaderSource(vs, 1, &vssc, nullptr);
        gl->glCompileShader(vs);
		printShaderInfoLog(vs);
        gl->glAttachShader(id_sencilni_program, vs);
	}

	{  // out_Color je barva, ki bo prišla do zaslona
        GLuint fs = gl->glCreateShader(GL_FRAGMENT_SHADER);
        std::string fss;
		fss += " #version 330					                \n";
        fss += " out vec4 out_Color;	                        \n";
		fss += " in vec3 gPos;						            \n";
        fss += " uniform vec4 DodajBarvo;						         \n";
        fss += " void main(){			                        \n";
        fss += "  out_Color=vec4(gPos.x,gPos.y ,0,1)+DodajBarvo;           \n";
		fss += " }						                        \n";
		std::cout << fss;
		const char *fssc = fss.c_str();
        gl->glShaderSource(fs, 1, &fssc, nullptr);
        gl->glCompileShader(fs);
		printShaderInfoLog(fs);
        gl->glAttachShader(id_sencilni_program, fs);
	}

    gl->glLinkProgram(id_sencilni_program);
	printProgramInfoLog(id_sencilni_program);
}

void WidgetOpenGLDraw::initializeGL() {
	// naložimo funkcije za OpenGL
    std::cout << "OpenGL context version: "<< context()->format().majorVersion() <<"." <<context()->format().minorVersion()<<std::endl;

//  ali uporabljate najnovejši QT6 ali 5
#if QT_VERSION >= 0x060000
    gl=QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(context());
#else
    gl=context()->versionFunctions<QOpenGLFunctions_3_3_Core>();
#endif


    if (!gl) {
        std::cerr << "Could not obtain required OpenGL context version";
        QApplication::exit(1);
    }


    std::cout << gl->glGetString(GL_VENDOR) << std::endl;
    std::cout << gl->glGetString(GL_VERSION) << std::endl;
    std::cout << gl->glGetString(GL_RENDERER) << std::endl;

	PrevediSencilnike();

	// glEnable(GL_DEPTH_TEST);		//v primeru, da rišemo več prekrivajočih
	// trikotnikov, želimo, da sprednje ne prekrijejo tisti, ki bi morali biti
	// odzadaj
	// glDisable(GL_CULL_FACE);		//rišemo obe lici trikotnikov

	// dejansko nosi lastnosti povezane z buffer (npr. stanje od
	// glEnableVertexAttribArray itd)
	// uporabno predvsem za večjo hitrost
    gl->glGenVertexArrays(1, &id_VAO_trikotnik);
    gl->glBindVertexArray(id_VAO_trikotnik);

	// naložimo trikotnik na GPU in določimo podatke
    //viewport -> območje izrisa v OpenGL je med -1 in 1 po obeh oseh

    //naredimo kocko,
    const glm::vec3 kocka[] = {

        //sprednja stranica
        glm::vec3( -0.2f  , -0.2f, 0.1f),
        glm::vec3( -0.2f  ,  0.2f, 0.1f),
        glm::vec3(  0.2f  ,  0.2f, 0.1f),

        glm::vec3( -0.2f  , -0.2f, 0.1f),
        glm::vec3(  0.2f  ,  0.2f, 0.1f),
        glm::vec3(  0.2f  , -0.2f, 0.1f),

        //desna stranica
        glm::vec3(  0.2f  , -0.2f, 0.1f),
        glm::vec3(  0.2f  ,  0.2f, 0.1f),
        glm::vec3(  0.2f  , -0.2f, 0.2f),

        glm::vec3(  0.2f  ,  0.2f, 0.1f),
        glm::vec3(  0.2f  ,  0.2f, 0.2f),
        glm::vec3(  0.2f  , -0.2f, 0.2f),

        //tla
        glm::vec3(  -0.2f  , -0.2f, 0.1f),
        glm::vec3(  -0.2f  ,  -0.2f, 0.2f),
        glm::vec3(  0.2f  , -0.2f, 0.2f),

        glm::vec3(  -0.2f  ,  -0.2f, 0.1f),
        glm::vec3(  0.2f  ,  -0.2f, 0.1f),
        glm::vec3(  0.2f  , -0.2f, 0.2f),

        //leva stranica
        glm::vec3(  -0.2f  , -0.2f, 0.1f),
        glm::vec3(  -0.2f  ,  -0.2f, 0.2f),
        glm::vec3(  -0.2f  , 0.2f, 0.2),

        glm::vec3(  -0.2f  ,  0.2f, 0.1f),
        glm::vec3(  -0.2f  ,  -0.2f, 0.2f),
        glm::vec3(  -0.2f  , 0.2f, 0.2f),

        //zadnja stranica
        glm::vec3(  -0.2f  , -0.2f, 0.2f),
        glm::vec3(  0.2f  ,  -0.2f, 0.2f),
        glm::vec3(  -0.2f  , 0.2f, 0.2),

        glm::vec3(  -0.2f  ,  0.2f, 0.2f),
        glm::vec3(  0.2f  ,  -0.2f, 0.2f),
        glm::vec3(  0.2f  , 0.2f, 0.2f),


        //vrh
        glm::vec3(  -0.2f  , 0.2f, 0.1f),
        glm::vec3(  -0.2f  ,  0.2f, 0.2f),
        glm::vec3(  0.2f  , 0.2f, 0.2f),

        glm::vec3(  -0.2f  ,  0.2f, 0.1f),
        glm::vec3(  0.2f  ,  0.2f, 0.1f),
        glm::vec3(  0.2f  , 0.2f, 0.2f),
                                       };



    const int st_trikotnikov = 12;

    //mogoce tu st_trikotnikov?
    gl->glGenBuffers(1, &id_buffer_trikotnik);//naredi n objektov shrani ime v buffer
    gl->glBindBuffer(GL_ARRAY_BUFFER, id_buffer_trikotnik);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 3 * st_trikotnikov , kocka, GL_STATIC_DRAW);
    gl->glEnableVertexAttribArray(0);  // uporabjamo: layout(location=0) in vec3 in_Pos;
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

    const unsigned int err = gl->glGetError();
	if (err != 0) {
		std::cerr << "OpenGL init napaka: " << err << std::endl;
	}
}


void skaliranje(){

}


void WidgetOpenGLDraw::resizeGL(int w, int h) {
    gl->glViewport(0, 0, w, h);
}

void WidgetOpenGLDraw::paintGL() {
	// počisti ozadje in globinski pomnilnik (za testiranje globine)
    gl->glClearColor(0.2f, 0.2f, 0.2f, 1);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gl->glBindVertexArray(id_VAO_trikotnik);
    gl->glUseProgram(id_sencilni_program);


    glm::mat4 I(1.0);//identiteta
    I = glm::rotate_slow(I, glm::radians(float(rotX)), glm::vec3(0, 0, 1));
    gl->glUniformMatrix4fv(gl->glGetUniformLocation(id_sencilni_program, "PVM"), 1, GL_FALSE, glm::value_ptr(I));

    //podamo barvo v obliki RGBA! v senčilniku fragmentov jo prištejemo!
    gl->glUniform4f(gl->glGetUniformLocation(id_sencilni_program, "DodajBarvo"), 1,dodajZelenoBarvo,0,0);

    //gl->glDrawArrays(GL_TRIANGLES, 0, 6);  // rištemo oglišča
    //gl->glDrawArrays(GL_TRIANGLES, 6, 6);  // rištemo oglišča
    //gl->glDrawArrays(GL_TRIANGLES, 12, 6);  // rištemo oglišča
    gl->glDrawArrays(GL_TRIANGLES, 0, 36);  // rištemo oglišča

    const unsigned int err = gl->glGetError();
	if (err != 0) {
		std::cerr << "OpenGL napaka: " << err << std::endl;
    }
}


void WidgetOpenGLDraw::Rotacija(){
    makeCurrent();// če bomo tukaj delali z OpenGL, je predtem potrebno klicati tole funkcijo
    // s tem povemo, da izvajamo OpenGL nad to površino (aplikacije imajo lahko več površin za izris!)
    // dialogi za odpiranje datotek imajo svoje površine in s tem svoj kontext!
    // http://doc.qt.io/qt-5/qopenglwidget.html#makeCurrent

	// lahko naložimo nove podatke
    //gl->glBufferData

    rotX+=10;
    update();
}



void WidgetOpenGLDraw::DodajZelenoBarvo(){
    makeCurrent();// če bomo tukaj delali z OpenGL, je predtem potrebno klicati tole funkcijo
    // s tem povemo, da izvajamo OpenGL nad to površino (aplikacije imajo lahko več površin za izris!)
    // dialogi za odpiranje datotek imajo svoje površine in s tem svoj kontext!
    // http://doc.qt.io/qt-5/qopenglwidget.html#makeCurrent

    // lahko naložimo nove podatke
    //gl->glBufferData

    dodajZelenoBarvo+=0.1;
    update();
}
