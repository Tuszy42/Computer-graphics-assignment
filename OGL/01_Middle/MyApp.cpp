#include "MyApp.h"
#include "GLUtils.hpp"

#include <GL/GLU.h>
#include <math.h>

#include "ObjParser_OGL3.h"

CMyApp::CMyApp(void)
{
	m_textureID = 0;
	m_textureID2 = 0;
	m_mesh = 0;
	m_mesh2 = 0;
	//kamera seg�dv�ltoz�
	x = 0;
	
	//Kezdeti sz�gek
	k1 = -135;
	k2 =  135;
	l1 = -180;
	l2 =  180;
}


CMyApp::~CMyApp(void)
{
}


GLuint CMyApp::GenTexture()
{
    unsigned char tex[256][256][3];
 
    for (int i=0; i<256; ++i)
        for (int j=0; j<256; ++j)
        {
			tex[i][j][0] = rand()%256;
			tex[i][j][1] = rand()%256;
			tex[i][j][2] = rand()%256;
        }
 
	GLuint tmpID;

	// gener�ljunk egy text�ra er�forr�s nevet
    glGenTextures(1, &tmpID);
	// aktiv�ljuk a most gener�lt nev� text�r�t
    glBindTexture(GL_TEXTURE_2D, tmpID);
	// t�lts�k fel adatokkal az...
    gluBuild2DMipmaps(  GL_TEXTURE_2D,	// akt�v 2D text�r�t
						GL_RGB8,		// a v�r�s, z�ld �s k�k csatorn�kat 8-8 biten t�rolja a text�ra
						256, 256,		// 256x256 m�ret� legyen
						GL_RGB,				// a text�ra forr�sa RGB �rt�keket t�rol, ilyen sorrendben
						GL_UNSIGNED_BYTE,	// egy-egy sz�nkopmonenst egy unsigned byte-r�l kell olvasni
						tex);				// �s a text�ra adatait a rendszermem�ria ezen szeglet�b�l t�lts�k fel
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// biline�ris sz�r�s kicsiny�t�skor
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// �s nagy�t�skor is
	glBindTexture(GL_TEXTURE_2D, 0);

	return tmpID;
}

bool CMyApp::Init()
{
	// t�rl�si sz�n legyen k�kes
	glClearColor(0.125f, 0.25f, 0.5f, 1.0f);

	glEnable(GL_CULL_FACE);		// kapcsoljuk be a hatrafele nezo lapok eldobasat
	glEnable(GL_DEPTH_TEST);	// m�lys�gi teszt bekapcsol�sa (takar�s)

	//
	// geometria letrehozasa
	//
	m_vb.AddAttribute(0, 3);
	m_vb.AddAttribute(1, 3);
	m_vb.AddAttribute(2, 2);

	m_vb.AddData(0, -50,  0, -50);
	m_vb.AddData(0,  50,  0, -50);
	m_vb.AddData(0, -50,  0,  50);
	m_vb.AddData(0,  50,  0,  50);

	m_vb.AddData(1, 0, 1, 0);
	m_vb.AddData(1, 0, 1, 0);
	m_vb.AddData(1, 0, 1, 0);
	m_vb.AddData(1, 0, 1, 0);
	 
	m_vb.AddData(2,  0,  0);
	m_vb.AddData(2, 10,  0);
	m_vb.AddData(2,  0, 10);
	m_vb.AddData(2, 10, 10);

	m_vb.AddIndex(1, 0, 2);
	m_vb.AddIndex(1, 2, 3);

	m_vb.InitBuffers();

	//
	// shaderek bet�lt�se
	//
	m_program.AttachShader(GL_VERTEX_SHADER, "dirLight.vert");
	m_program.AttachShader(GL_FRAGMENT_SHADER, "dirLight.frag");

	m_program.BindAttribLoc(0, "vs_in_pos");
	m_program.BindAttribLoc(1, "vs_in_normal");
	m_program.BindAttribLoc(2, "vs_in_tex0");

	if ( !m_program.LinkProgram() )
	{
		return false;
	}

	//
	// egy�b inicializ�l�s
	//

	m_camera.SetProj(45.0f, 640.0f/480.0f, 0.01f, 1000.0f);

	// text�ra bet�lt�se
	m_textureID = TextureFromFile("floor.png");
	m_textureID2 = TextureFromFile("wire.png");

	// mesh bet�lt�s
	m_mesh = ObjParser::parse("vegtag.obj");
	m_mesh->initBuffers();

	m_mesh2 = ObjParser::parse("gomb.obj");
	m_mesh2->initBuffers();

	return true;
}

void CMyApp::Clean()
{
	glDeleteTextures(1, &m_textureID);

	m_program.Clean();
}

void CMyApp::Update()
{
	static Uint32 last_time = SDL_GetTicks();
	float delta_time = (SDL_GetTicks() - last_time)/1000.0f;

	m_camera.Update(delta_time);

	last_time = SDL_GetTicks();
}


void CMyApp::Render()
{
	// t�r�lj�k a frampuffert (GL_COLOR_BUFFER_BIT) �s a m�lys�gi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Kamera mozgat�s k�rp�ly�n(k�t szinus), a test k�zep�be n�z, y a felfel� mutat� ir�ny
	m_camera.SetView(glm::vec3(sinf(2*3.1415*x)*40,15,cosf(2*3.1415*x)*40),glm::vec3(0,15,0),glm::vec3(0,1,0));

	//Talaj kirajzol�sa
	m_program.On();

	glm::mat4 matWorld = glm::mat4(1.0f);
	glm::mat4 matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	glm::mat4 mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_program.SetTexture("texImage", 0, m_textureID);

	// kapcsoljuk be a VAO-t (a VBO j�n vele egy�tt)
	m_vb.On();

	m_vb.DrawIndexed(GL_TRIANGLES, 0, 6, 0);

	m_vb.Off();

	// shader kikapcsolasa
	m_program.Off();

	// 2. program
	DrawMan();
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	//V�gtagok mozgat�s, meg vannak adva a maximum �s minimum sz�gek
	if(key.keysym.sym == SDLK_w && k1 <= -35){
		k1 += 5;
	}else if(key.keysym.sym == SDLK_s && k1 >= -155){
		k1 -= 5;
	}else if(key.keysym.sym == SDLK_d && k2 <= 155){
		k2 += 5;
	}else if(key.keysym.sym == SDLK_e && k2 >= 35){
		k2 -= 5;
	}else if(key.keysym.sym == SDLK_r && l1 <= -65){
		l1 += 5;
	}else if(key.keysym.sym == SDLK_f && l1 >= -195){
		l1 -= 5;
	}else if(key.keysym.sym == SDLK_g && l2 <= 195){
		l2 += 5;
	}else if(key.keysym.sym == SDLK_t && l2 >= 65){
		l2 -= 5;
	}
	//Ez mozgatja a kamer�t a k�rp�ly�n
	if(key.keysym.sym == SDLK_LEFT){
		x -= 0.01f;
	}else if(key.keysym.sym == SDLK_RIGHT){
		x += 0.01f;
	}
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
}

void CMyApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
}

// a k�t param�terbe az �j ablakm�ret sz�less�ge (_w) �s magass�ga (_h) tal�lhat�
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);

	m_camera.Resize(_w, _h);
}

void CMyApp::DrawMan(){
	m_program.On();
	//T�rzs
	glm::mat4 matWorld = glm::translate<float>(0, 7, 0) * glm::scale<float>(2,1,2);
	glm::mat4 matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	glm::mat4 mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_program.SetTexture("texImage", 0, m_textureID2);
	

	m_mesh->draw();

	//Fej
	matWorld = glm::translate<float>(0, 18.5f, 0) * glm::scale<float>(1.5f,1.5f,1.5f);
	matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_mesh2->draw();


	//KAR1
	
	//felkar
	matWorld =   glm::translate<float>(2, 16, 0) * glm::rotate<float>(k1,0,0,1) * glm::scale<float>(0.4f,0.3f,0.4f);
	matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_mesh->draw();
	//k�ny�k
	matWorld =   glm::translate<float>(sinf(DegToRad(-k1))*3.5f + 2, cosf(DegToRad(-k1))*3.5f + 16, 0) * glm::scale<float>(0.5f,0.5f,0.5f);
	matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_mesh2->draw();

	//alkar
	matWorld =   glm::translate<float>(sinf(DegToRad(-k1))*4 + 2, cosf(DegToRad(-k1))*4 + 16, 0) * glm::rotate<float>(k1-45,0,0,1) * glm::scale<float>(0.4f,0.3f,0.4f);
	matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_mesh->draw();

	//KAR2
	//felkar
	m_program.SetUniform( "La", glm::vec4(0.5f, 0.5f, 0.5f,1));
	matWorld =   glm::translate<float>(-2, 16, 0) * glm::rotate<float>(k2,0,0,1) * glm::scale<float>(0.4f,0.3f,0.4f);
	matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_mesh->draw();
	//k�ny�k
	matWorld =   glm::translate<float>(sinf(DegToRad(-k2))*3.5f - 2, cosf(DegToRad(-k2))*3.5f + 16, 0) * glm::scale<float>(0.5f,0.5f,0.5f);
	matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_mesh2->draw();

	//alkar
	matWorld =   glm::translate<float>(sinf(DegToRad(-k2))*4 - 2, cosf(DegToRad(-k2))*4 + 16, 0) * glm::rotate<float>(k2+45,0,0,1) * glm::scale<float>(0.4f,0.3f,0.4f);
	matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_mesh->draw();
	
	//L�B1

	//comb
	matWorld =   glm::translate<float>(1.5f, 7, 0) * glm::rotate<float>(l1,0,0,1) * glm::scale<float>(0.4f,0.3f,0.4f);
	matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_mesh->draw();
	//t�rd
	matWorld =   glm::translate<float>(sinf(DegToRad(-l1))*3.5f + 1.5f, cosf(DegToRad(-l1))*3.5f + 7, 0) * glm::scale<float>(0.5f,0.5f,0.5f);
	matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_mesh2->draw();

	//v�dli
	matWorld =   glm::translate<float>(sinf(DegToRad(-l1))*4 + 1.5f, cosf(DegToRad(-l1))*4 + 7, 0) * glm::rotate<float>(180,0,0,1) * glm::scale<float>(0.4f,0.3f,0.4f);
	matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_mesh->draw();

	//L�B2

	//comb
	matWorld =   glm::translate<float>(-1.5f, 7, 0) * glm::rotate<float>(l2,0,0,1) * glm::scale<float>(0.4f,0.3f,0.4f);
	matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_mesh->draw();
	//t�rd
	matWorld =   glm::translate<float>(sinf(DegToRad(-l2))*3.5f - 1.5f, cosf(DegToRad(-l2))*3.5f + 7, 0) * glm::scale<float>(0.5f,0.5f,0.5f);
	matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_mesh2->draw();

	//v�dli
	matWorld =   glm::translate<float>(sinf(DegToRad(-l2))*4 - 1.5f, cosf(DegToRad(-l2))*4 + 7, 0) * glm::rotate<float>(180,0,0,1) * glm::scale<float>(0.4f,0.3f,0.4f);
	matWorldIT = glm::transpose( glm::inverse( matWorld ) );
	mvp = m_camera.GetViewProj() *matWorld;

	m_program.SetUniform( "world", matWorld );
	m_program.SetUniform( "worldIT", matWorldIT );
	m_program.SetUniform( "MVP", mvp );
	m_program.SetUniform( "eye_pos", m_camera.GetEye() );

	m_mesh->draw();
	

	m_program.Off();
}

//Seg�df�ggv�ny, fokb�l radi�nba v�lt
float CMyApp::DegToRad(float deg){
	return deg * (M_PI/180);
}