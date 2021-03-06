/*
    Pony48 source - webcam.cpp
    Copyright (c) 2014 Mark Hutcheson
*/

#include "webcam.h"

Webcam::Webcam()
{
#ifndef USE_VIDEOINPUT
	m_VideoCap = NULL;
#elif !defined(NO_WEBCAM)
	m_device = 0;
#endif
	m_hTex = 0;
	m_curFrame = NULL;
	m_iWidth = m_iHeight = -1;
	mirror = true;
	use = true;
}

Webcam::~Webcam()
{
	_clear();
}

void Webcam::draw(float32 height, Point ptCenter)
{
	if(!use) return;
	if(m_hTex)
	{
		// tell opengl to use the generated texture
		glBindTexture(GL_TEXTURE_2D, m_hTex);
		
		float32 width = height * ((float32)m_iWidth / (float32)m_iHeight);
		width /= 2.0;
		height /= 2.0;

		const GLfloat vertexData[] =
		{
			ptCenter.x - width, ptCenter.y + height, // upper left
			ptCenter.x + width, ptCenter.y + height, // upper right
			ptCenter.x - width, ptCenter.y - height, // lower left
			ptCenter.x + width, ptCenter.y - height, // lower right
		};
		
		float left, right, top, bot;
		if(mirror)
		{
			left = 1;
			right = 0;
		}
		else
		{
			left = 0;
			right = 1;
		}
		
#ifdef USE_VIDEOINPUT
		top = 1;
		bot = 0;
#else
		top = 0;
		bot = 1;
#endif
		
		const GLfloat texCoords[] =
		{
			left, top, // upper left
			right, top, // upper right
			left, bot, // lower left
			right, bot, // lower right
		};
		glVertexPointer(2, GL_FLOAT, 0, &vertexData);
		glTexCoordPointer(2, GL_FLOAT, 0, &texCoords);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

void Webcam::getNewFrame()
{
	if(!use) return;
#ifdef USE_VIDEOINPUT
	if(m_curFrame == NULL)
		m_curFrame = new unsigned char [VI.getSize(m_device)];
		
	if(VI.isFrameNew(m_device))
	{
		VI.getPixels(m_device, m_curFrame, false, false);
		m_iHeight = VI.getHeight(m_device);
		m_iWidth = VI.getWidth(m_device);
		if(m_iHeight > 0 && m_iWidth > 0)
		{
			//Create OpenGL texture out of this data
			if(!m_hTex)
				glGenTextures(1, &m_hTex);
			glBindTexture(GL_TEXTURE_2D, m_hTex);
			
			int mode, modeflip;
			mode = GL_RGB;
			modeflip = GL_BGR;			glTexImage2D(GL_TEXTURE_2D, 0, mode, m_iWidth, m_iHeight, 0, modeflip, GL_UNSIGNED_BYTE, m_curFrame);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		}
	}
#elif !defined(NO_WEBCAM)
	if(m_VideoCap == NULL || !m_VideoCap->isOpened())
		return;
	if(m_curFrame == NULL)
		m_curFrame = new cv::Mat;
	if(m_VideoCap->read(*m_curFrame))	//Grab frame from webcam (can take some time)
	{
		m_iHeight = m_curFrame->rows;
		m_iWidth = m_curFrame->cols;
		if(m_iHeight > 0 && m_iWidth > 0)
		{
			//Create OpenGL texture out of this data
			if(!m_hTex)
				glGenTextures(1, &m_hTex);
			glBindTexture(GL_TEXTURE_2D, m_hTex);
			
			int mode, modeflip;
			mode = GL_RGB;
			modeflip = GL_BGR;
			glTexImage2D(GL_TEXTURE_2D, 0, mode, m_iWidth, m_iHeight, 0, modeflip, GL_UNSIGNED_BYTE, m_curFrame->data);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		}
	}
#endif
}

void Webcam::open(int device)
{
	if(!use) return;
	_clear();
#ifdef USE_VIDEOINPUT
	m_device = device;
	VI.setupDevice(m_device);
#elif !defined(NO_WEBCAM)
	//Open the webcam with OpenCV
	m_VideoCap = new cv::VideoCapture(device);
	if(!m_VideoCap->isOpened())	
	{
		m_VideoCap->open(device);
		if(!m_VideoCap->isOpened())	
		{
			errlog << "Unable to open webcam " << device << endl;
			m_VideoCap->release();
			delete m_VideoCap;
			m_VideoCap = NULL;
		}
		else
			getNewFrame();	//Grab a new frame now while we're loading, because there seems to be a lag on getting the first frame
	}
	else
		getNewFrame();	//Grab a new frame now while we're loading, because there seems to be a lag on getting the first frame
#endif
}

void Webcam::_clear()
{
#ifdef USE_VIDEOINPUT
	VI.stopDevice(m_device);
#elif !defined(NO_WEBCAM)
	if(m_VideoCap)
	{
		m_VideoCap->release();
		delete m_VideoCap;
	}
	m_VideoCap = NULL;
#endif
	if(m_curFrame)
		delete m_curFrame;
	m_curFrame = NULL;
	if(m_hTex)
		glDeleteTextures(1, &m_hTex);
	m_hTex = 0;
	m_iWidth = m_iHeight = -1;
}

bool Webcam::isOpen()
{
	if(!use) return false;
#ifdef USE_VIDEOINPUT
	return (VI.isDeviceSetup(m_device));
#elif !defined(NO_WEBCAM)
	return (m_VideoCap != NULL && m_VideoCap->isOpened());
#else
  return false;
#endif
}

bool Webcam::saveFrame(string sFilename, bool bMirror)
{
	if(!use) return false;
	if(!m_curFrame) return false;
#ifdef USE_VIDEOINPUT
	FIBITMAP* bmp = FreeImage_ConvertFromRawBits(m_curFrame, m_iWidth, m_iHeight, m_iWidth * 3, 24, 0x0000FF, 0x00FF00, 0xFF0000, true);
#elif !defined(NO_WEBCAM)
	FIBITMAP* bmp = FreeImage_ConvertFromRawBits(m_curFrame->data, m_iWidth, m_iHeight, m_iWidth * 3, 24, 0x0000FF, 0x00FF00, 0xFF0000, true);
#else
  FIBITMAP* bmp = NULL;
#endif
	if(!bmp) return false;
	if(bMirror)
		FreeImage_FlipHorizontal(bmp);
#ifdef USE_VIDEOINPUT
	FreeImage_FlipVertical(bmp);
#endif
	bool bRet = FreeImage_Save(FIF_JPEG, bmp, sFilename.c_str());
	FreeImage_Unload(bmp);
	return bRet;
}
































