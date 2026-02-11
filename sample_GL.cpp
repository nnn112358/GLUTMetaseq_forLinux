/*---------------------------------------------------------------------------
           MQOファイルの3Dモデルを読み込みOpenGLで表示するサンプル
           デバッグ機能強化版
---------------------------------------------------------------------------*/

#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif

#include "GLMetaseq.h"	// モデルローダ


// グローバル変数
int g_mouseX = 0;	// マウスX座標
int g_mouseY = 0;	// マウスY座標
int g_rotX = 0;		// X軸周りの回転
int	g_rotY = 0;		// Y軸周りの回転

MQO_MODEL g_mqoModel = NULL;	// MQOモデル
int g_modelLoaded = 0;			// モデル読み込み成功フラグ
int g_frameCount = 0;			// フレームカウンタ

// FPS計測
int g_fpsFrameCount = 0;		// FPS計測用フレームカウンタ
int g_fpsLastTime = 0;			// 前回FPS計測時刻(ms)
float g_currentFPS = 0.0f;		// 現在のFPS値

// ウィンドウサイズ
int g_windowWidth = 640;
int g_windowHeight = 480;

// ワイヤフレームモード (0=Fill, 1=Wireframe)
int g_wireframeMode = 0;

// デバッグオーバーレイ表示 (1=ON, 0=OFF)
int g_debugOverlay = 1;

// デバッグフラグ (1=ON, 0=OFF)
#define DEBUG_ENABLED 1

// プロトタイプ宣言
void mySetLight(void);
void Draw(void);
void Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void updateFPS(void);
void drawBitmapString(float x, float y, const char *str);
void drawHUD(void);
void drawDebugGrid(float size, float step);


/*=========================================================================
  デバッグ用: OpenGLエラーチェック
=========================================================================*/
void checkGLError(const char *location)
{
#if DEBUG_ENABLED
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		const char *errStr;
		switch (err) {
			case GL_INVALID_ENUM:      errStr = "GL_INVALID_ENUM"; break;
			case GL_INVALID_VALUE:     errStr = "GL_INVALID_VALUE"; break;
			case GL_INVALID_OPERATION: errStr = "GL_INVALID_OPERATION"; break;
			case GL_STACK_OVERFLOW:    errStr = "GL_STACK_OVERFLOW"; break;
			case GL_STACK_UNDERFLOW:   errStr = "GL_STACK_UNDERFLOW"; break;
			case GL_OUT_OF_MEMORY:     errStr = "GL_OUT_OF_MEMORY"; break;
			default:                   errStr = "UNKNOWN"; break;
		}
		printf("[GL ERROR] %s: %s (0x%04X)\n", location, errStr, err);
	}
#endif
}


/*=========================================================================
  デバッグ用: OpenGL情報の表示
=========================================================================*/
void printGLInfo(void)
{
#if DEBUG_ENABLED
	printf("============================================\n");
	printf("  OpenGL Debug Info\n");
	printf("============================================\n");
	printf("  GL_VENDOR:   %s\n", glGetString(GL_VENDOR));
	printf("  GL_RENDERER: %s\n", glGetString(GL_RENDERER));
	printf("  GL_VERSION:  %s\n", glGetString(GL_VERSION));

	GLint maxTexSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
	printf("  Max Texture Size: %d\n", maxTexSize);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	printf("  Viewport: %d, %d, %d, %d\n", viewport[0], viewport[1], viewport[2], viewport[3]);

	GLint depthBits;
	glGetIntegerv(GL_DEPTH_BITS, &depthBits);
	printf("  Depth Buffer Bits: %d\n", depthBits);

	GLint colorBits[4];
	glGetIntegerv(GL_RED_BITS, &colorBits[0]);
	glGetIntegerv(GL_GREEN_BITS, &colorBits[1]);
	glGetIntegerv(GL_BLUE_BITS, &colorBits[2]);
	glGetIntegerv(GL_ALPHA_BITS, &colorBits[3]);
	printf("  Color Bits: R=%d G=%d B=%d A=%d\n", colorBits[0], colorBits[1], colorBits[2], colorBits[3]);

	printf("============================================\n");
	checkGLError("printGLInfo");
#endif
}


/*=========================================================================
  デバッグ用: テスト三角形の描画 (OpenGL基本動作確認)
=========================================================================*/
void drawDebugTriangle(void)
{
#if DEBUG_ENABLED
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	// 画面左下にカラフルな三角形を描画（OpenGL動作確認用）
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 0.0f, 0.0f);  // 赤
		glVertex3f(-0.9f, -0.9f, 0.0f);
		glColor3f(0.0f, 1.0f, 0.0f);  // 緑
		glVertex3f(-0.7f, -0.9f, 0.0f);
		glColor3f(0.0f, 0.0f, 1.0f);  // 青
		glVertex3f(-0.8f, -0.7f, 0.0f);
	glEnd();

	// 画面右上にテキスト的な目印（白い小さな四角）
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
		glVertex3f(0.85f, 0.85f, 0.0f);
		glVertex3f(0.95f, 0.85f, 0.0f);
		glVertex3f(0.95f, 0.95f, 0.0f);
		glVertex3f(0.85f, 0.95f, 0.0f);
	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glPopMatrix();
	glPopAttrib();

	checkGLError("drawDebugTriangle");
#endif
}


/*=========================================================================
  デバッグ用: 座標軸の描画
=========================================================================*/
void drawDebugAxes(float length)
{
#if DEBUG_ENABLED
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(2.0f);

	glBegin(GL_LINES);
		// X軸 (赤)
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(length, 0.0f, 0.0f);
		// Y軸 (緑)
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, length, 0.0f);
		// Z軸 (青)
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, length);
	glEnd();

	glPopAttrib();
	checkGLError("drawDebugAxes");
#endif
}


/*=========================================================================
  FPS計測・更新 (1秒ごとにコンソール出力)
=========================================================================*/
void updateFPS(void)
{
#if DEBUG_ENABLED
	g_fpsFrameCount++;
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = currentTime - g_fpsLastTime;

	if (elapsed >= 1000) {
		g_currentFPS = g_fpsFrameCount * 1000.0f / elapsed;
		printf("[FPS] %.1f fps\n", g_currentFPS);
		g_fpsFrameCount = 0;
		g_fpsLastTime = currentTime;
	}
#endif
}


/*=========================================================================
  ビットマップ文字列の描画
=========================================================================*/
void drawBitmapString(float x, float y, const char *str)
{
	glRasterPos2f(x, y);
	for (const char *c = str; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c);
	}
}


/*=========================================================================
  HUD（ヘッドアップディスプレイ）描画
=========================================================================*/
void drawHUD(void)
{
#if DEBUG_ENABLED
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// 2D正射影に切り替え
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, g_windowWidth, 0, g_windowHeight, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	// 半透明黒帯を画面上部に描画
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
	float bandHeight = 54.0f;
	float bandTop = (float)g_windowHeight;
	float bandBottom = bandTop - bandHeight;
	glBegin(GL_QUADS);
		glVertex2f(0.0f, bandBottom);
		glVertex2f((float)g_windowWidth, bandBottom);
		glVertex2f((float)g_windowWidth, bandTop);
		glVertex2f(0.0f, bandTop);
	glEnd();
	glDisable(GL_BLEND);

	// 緑色テキストで情報表示
	glColor3f(0.0f, 1.0f, 0.0f);
	char buf[256];

	// 1行目: FPS / フレームカウント
	sprintf(buf, "FPS: %.1f  Frame: %d", g_currentFPS, g_frameCount);
	drawBitmapString(8.0f, bandTop - 16.0f, buf);

	// 2行目: 回転角 / モデル状態 / ワイヤフレーム
	sprintf(buf, "RotX: %d  RotY: %d  Model: %s  Wire: %s",
		g_rotX, g_rotY,
		g_modelLoaded ? "OK" : "NONE",
		g_wireframeMode ? "ON" : "OFF");
	drawBitmapString(8.0f, bandTop - 32.0f, buf);

	// 3行目: マウス座標
	sprintf(buf, "Mouse: (%d, %d)  Overlay: %s",
		g_mouseX, g_mouseY,
		g_debugOverlay ? "ON" : "OFF");
	drawBitmapString(8.0f, bandTop - 48.0f, buf);

	// 行列復元
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
	checkGLError("drawHUD");
#endif
}


/*=========================================================================
  デバッグ用: XZ平面のグリッド描画
=========================================================================*/
void drawDebugGrid(float size, float step)
{
#if DEBUG_ENABLED
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(1.0f);
	glColor3f(0.4f, 0.4f, 0.4f);

	float half = size / 2.0f;
	glBegin(GL_LINES);
	for (float i = -half; i <= half; i += step) {
		// X方向の線
		glVertex3f(i, 0.0f, -half);
		glVertex3f(i, 0.0f, half);
		// Z方向の線
		glVertex3f(-half, 0.0f, i);
		glVertex3f(half, 0.0f, i);
	}
	glEnd();

	glPopAttrib();
	checkGLError("drawDebugGrid");
#endif
}


// 照明の設定を行う関数
void mySetLight(void)
{
	GLfloat light_diffuse[]  = { 0.9f, 0.9f, 0.9f, 1.0f };	// 拡散反射光
	GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };	// 鏡面反射光
	GLfloat light_ambient[]  = { 0.3f, 0.3f, 0.3f, 1.0f };	// 環境光
	GLfloat light_position[] = { 0.0f, 200.0f, 200.0f, 1.0f };// 位置 (カメラ前方上方に変更)

	// 照明の設定
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHT0);

	checkGLError("mySetLight");
}


// 描画関数
void Draw(void)
{
	g_frameCount++;
	updateFPS();

	// glClearColorはglClearの前に設定する（修正）
	glClearColor(0.0f, 0.0f, 0.2f, 1.0f);					// 背景色（暗い青）
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// クリア

	checkGLError("Draw: after clear");

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	// モデルビュー行列をリセット（追加）

	glEnable(GL_DEPTH_TEST);		// 隠面処理の適用
	mySetLight();					// 照明の設定
	glEnable(GL_LIGHTING);			// 照明ON

	checkGLError("Draw: after lighting setup");

	// ワイヤフレームモード設定
	if (g_wireframeMode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glPushMatrix();
		glTranslatef(0.0f, -100.0f, -400.0f);	// 平行移動
		glRotatef((float)g_rotX, 1, 0, 0);		// X回転
		glRotatef((float)g_rotY, 0, 1, 0);		// Y回転

		// デバッグ: 座標軸を描画
		if (g_debugOverlay) {
			drawDebugAxes(100.0f);
			drawDebugGrid(400.0f, 50.0f);
		}

		if (g_modelLoaded && g_mqoModel != NULL) {
			mqoCallModel(g_mqoModel);			// MQOモデルのコール
			checkGLError("Draw: after mqoCallModel");
		}
	glPopMatrix();

	// ワイヤフレームをリセット（HUD等に影響しないように）
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	// デバッグ: テスト三角形を描画（OpenGL動作確認）
	if (g_debugOverlay) {
		drawDebugTriangle();
	}

	// デバッグ: HUD描画
	if (g_debugOverlay) {
		drawHUD();
	}

	// デバッグ: 最初の数フレームだけログ出力
#if DEBUG_ENABLED
	if (g_frameCount <= 3) {
		printf("[Frame %d] Draw completed. modelLoaded=%d, mqoModel=%p, rotX=%d, rotY=%d\n",
			g_frameCount, g_modelLoaded, (void*)g_mqoModel, g_rotX, g_rotY);
	}
#endif

	glutSwapBuffers();
	checkGLError("Draw: after swap");
}


// ウィンドウ変形時に呼ばれる関数
void Reshape(int w, int h)
{
	double	znear = 10;
	double	fovy = 50;
	double	zfar = 10000;

#if DEBUG_ENABLED
	printf("[Reshape] w=%d, h=%d\n", w, h);
#endif

	if (h == 0) h = 1;  // ゼロ除算防止

	g_windowWidth = w;
	g_windowHeight = h;

	// ビューポート設定
	glViewport(0, 0, w, h);

	// 射影設定
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (double)w / h, znear, zfar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	checkGLError("Reshape");
}


// キーイベント関数
void Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 'q':
		case 'Q':
		case 0x1b:
			exit(0);
		case 'd':
		case 'D':
			// デバッグ情報の再表示
			printGLInfo();
			printf("[Debug] modelLoaded=%d, mqoModel=%p, frameCount=%d\n",
				g_modelLoaded, (void*)g_mqoModel, g_frameCount);
			printf("[Debug] rotX=%d, rotY=%d\n", g_rotX, g_rotY);
			checkGLError("Keyboard debug dump");
			break;
		case 'r':
		case 'R':
			// 回転リセット
			g_rotX = 0;
			g_rotY = 0;
			printf("[Debug] Rotation reset.\n");
			break;
		case 'w':
		case 'W':
			// ワイヤフレーム切替
			g_wireframeMode = !g_wireframeMode;
			printf("[Debug] Wireframe: %s\n", g_wireframeMode ? "ON" : "OFF");
			break;
		case 't':
		case 'T':
			// デバッグオーバーレイ一括切替
			g_debugOverlay = !g_debugOverlay;
			printf("[Debug] Debug overlay: %s\n", g_debugOverlay ? "ON" : "OFF");
			break;
		case 'h':
		case 'H':
			// キーボードヘルプ表示
			printf("============================================\n");
			printf("  Keyboard Help\n");
			printf("============================================\n");
			printf("  Q / ESC : Quit\n");
			printf("  D       : Dump debug info\n");
			printf("  R       : Reset rotation\n");
			printf("  W       : Toggle wireframe mode\n");
			printf("  T       : Toggle debug overlay\n");
			printf("  H       : Show this help\n");
			printf("============================================\n");
			break;
		default:
			break;
	}
}


// マウスイベント関数
void Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		g_mouseX = x;
		g_mouseY = y;
#if DEBUG_ENABLED
		const char *btnName;
		switch (button) {
			case GLUT_LEFT_BUTTON:   btnName = "LEFT"; break;
			case GLUT_MIDDLE_BUTTON: btnName = "MIDDLE"; break;
			case GLUT_RIGHT_BUTTON:  btnName = "RIGHT"; break;
			default:                 btnName = "UNKNOWN"; break;
		}
		printf("[Mouse] %s click at (%d, %d)\n", btnName, x, y);
#endif
	}
}


// マウスドラッグの処理関数
void Motion(int x, int y)
{
	int xd, yd;

	// マウス移動量の算出
	xd = x - g_mouseX;
	yd = y - g_mouseY;

	// 回転量の設定
	g_rotX += yd;
	g_rotY += xd;

	// マウス座標の更新
	g_mouseX = x;
	g_mouseY = y;

#if DEBUG_ENABLED
	// 100フレームごとにドラッグ情報をログ（スパム防止）
	if (g_frameCount % 100 == 0) {
		printf("[Motion] drag pos=(%d,%d) rot=(%d,%d)\n", x, y, g_rotX, g_rotY);
	}
#endif
}


// 終了処理関数
void Quit(void)
{
	printf("[Quit] Cleaning up...\n");
	if (g_modelLoaded && g_mqoModel != NULL) {
		mqoDeleteModel(g_mqoModel);
	}
	mqoCleanup();
	printf("[Quit] Done.\n");
}


// ファイル存在チェック
int fileExists(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (fp) {
		fclose(fp);
		return 1;
	}
	return 0;
}


// main関数
int main(int argc, char *argv[])
{
	printf("============================================\n");
	printf("  MQO Loader for OpenGL (Debug Mode)\n");
	printf("============================================\n");

	// 初期化
	glutInit(&argc, argv);										// OpenGL初期化
	glutInitWindowPosition(100, 50);							// ウィンドウの表示位置
	glutInitWindowSize(640, 480);								// ウィンドウのサイズ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);	// ディスプレイ設定
	glutCreateWindow("MQO Loader for OpenGL [DEBUG]");			// ウィンドウの生成

	// OpenGL情報の表示
	printGLInfo();

	// モデルファイルの存在チェック
	const char *modelFile = "ninja.mqo";
	printf("\n[Init] Checking model file: %s\n", modelFile);
	if (fileExists(modelFile)) {
		printf("[Init] OK - File '%s' found.\n", modelFile);
	} else {
		printf("[Init] ERROR - File '%s' NOT FOUND!\n", modelFile);
		printf("[Init] Current directory may be wrong.\n");
		printf("[Init] Trying to continue anyway...\n");
	}

	// モデルを表示する準備
	printf("[Init] Calling mqoInit()...\n");
	mqoInit();
	checkGLError("after mqoInit");
	printf("[Init] mqoInit() done.\n");

	printf("[Init] Loading model '%s'...\n", modelFile);
	g_mqoModel = mqoCreateModel((char*)modelFile, 1.0);
	checkGLError("after mqoCreateModel");

	if (g_mqoModel != NULL) {
		g_modelLoaded = 1;
		printf("[Init] OK - Model loaded successfully! (ptr=%p)\n", (void*)g_mqoModel);
		printf("[Init]   objnum = %d\n", g_mqoModel->objnum);
		for (int i = 0; i < g_mqoModel->objnum; i++) {
			printf("[Init]   obj[%d]: name='%s', visible=%d, matnum=%d\n",
				i,
				g_mqoModel->obj[i].objname,
				g_mqoModel->obj[i].isVisible,
				g_mqoModel->obj[i].matnum);
		}
	} else {
		g_modelLoaded = 0;
		printf("[Init] ERROR - mqoCreateModel returned NULL!\n");
		printf("[Init] Model loading FAILED. Only debug geometry will be shown.\n");
	}

	// 終了処理関数の設定
	atexit(Quit);

	// コールバック関数の設定
	glutDisplayFunc(Draw);			// 描画処理関数の設定
	glutIdleFunc(Draw);				// アイドル時の処理関数の設定
	glutReshapeFunc(Reshape);		// ウィンドウ変形時の処理を行う関数の設定
	glutKeyboardFunc(Keyboard);		// キー入力時の処理関数の設定
	glutMouseFunc(Mouse);			// マウス入力時の処理関数の設定
	glutMotionFunc(Motion);			// マウスドラッグ時の処理関数の設定

	printf("\n[Init] Entering main loop.\n");
	printf("[Init] Keys: Q/ESC=quit, D=debug info, R=reset rotation\n");
	printf("[Init]        W=wireframe, T=toggle overlay, H=help\n");
	printf("============================================\n\n");

	// イベント待ちの無限ループに入る
	glutMainLoop();

	return 0;
}
