#include <GLUT/glut.h>  // Macの場合のヘッダ
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "matrix.h"

#define _USE_MATH_DEFINES
#include <math.h>
#define pi 3.14159265358979
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define swap(a, b) do { \
    double t = (a);     \
    a = b;              \
    b = t;              \
} while( 0 );

int WINDOW_WIDTH = 500;     // ウィンドウの横幅
int WINDOW_HEIGHT = 500;    // ウィンドウの高さ

// 変形に使う変数
double TIME = 0.0;
double SCALE = 1.0;

//文字描画用
int text_list;
char point_char[20];
char point_char2[30];
char wind_char[20];
char wind_char2[30];
char wind_char3[10];
char high_char[20];
char high_char2[30];
void DRAW_STRING(int x, int y, char *string, void *font);
void DISPLAY_TEXT(int x, int y, char *string);

//得点
int points = 0, high_score = 0;

//風力
double w1 = 0, w2 = 0;

// マウスの状態を表す変数
double xc = 0;

//ゴミの状態を表す変数
int shoot = 0;

//モードを表す変数
int mode = 1;

// カメラの位置
static const float CAMERA_POS[3] = { 0.0f, -40.0f, 30.0f };

//runge_kutta用の変数
matrix x_0,y_0,z_0;
matrix x,y,z;

// OpenGLの初期化関数
void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);

    // ライティング機能の有効化
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // 法線の長さが常に1になるようにする
    // glScalefを使う時に特に必要になる
    glEnable(GL_NORMALIZE);

    // ライトの位置の設定
    // 好みがなければカメラの位置と同じにすると良い
    float light_position[4] = { 3.0, 4.0, 100.0, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    float light_position2[4] = { 3.0, -4.0, 100.0, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, light_position2);

    // シェーディングのやり方
    // GL_FLAT: フラットシェーディング
    // GL_SMOOTH: スムースシェーディング
    glShadeModel(GL_SMOOTH);

    // マテリアルの初期値を設定
    float ambient[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    float diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    float specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    float shininess = 32.0f;
    glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);

    // --- 自分独自の初期化を追加する場合はこの下に記述する ---
    srand((unsigned)time(NULL)); //乱数のシード
    // --- 自分独自の初期化を追加する場合はこの上に記述する ---
}

//--------------------------------------------------------
// 文字描画
//--------------------------------------------------------
void DISPLAY_TEXT(int x, int y, char *string){
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    glPushAttrib(GL_ENABLE_BIT);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 100, 0, 100);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(1.0, 1.0, 1.0);
    glCallList(text_list);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glMatrixMode(GL_MODELVIEW);
    text_list=glGenLists(1);
    glNewList(text_list,GL_COMPILE);

    DRAW_STRING(x, y, string , GLUT_BITMAP_TIMES_ROMAN_24);
    glEndList();

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}
void DRAW_STRING(int x, int y, char *string, void *font){
    int len, i;
    glRasterPos2f(x, y);
    len = (int) strlen(string);
    for (i = 0; i < len; i++){
        glutBitmapCharacter(font, string[i]);
    }
}
void DRAW_POINTS(int points) {
    strcpy(point_char2, "point = ");
    sprintf(point_char, "%d", points);
    strcat(point_char2, point_char);
    DISPLAY_TEXT(5, 95, point_char2);
}
void DRAW_HIGHSCORE(int high_score){
    strcpy(high_char2, "highscore = ");
    sprintf(high_char, "%d", high_score);
    strcat(high_char2, high_char);
    DISPLAY_TEXT(5, 90, high_char2);
}
void DRAW_WIND(double w) {
    strcpy(wind_char2, "wind = ");
    if(w >= 0) {
        sprintf(wind_char, "%f", w);
        strcpy(wind_char3, "<----");
    } else {
        sprintf(wind_char, "%f", -w);
        strcpy(wind_char3, "---->");
    }
    strcat(wind_char2, wind_char);
    DISPLAY_TEXT(33, 85, wind_char2);
    DISPLAY_TEXT(43, 80, wind_char3);
}
//-------------------------------

//地面
void ground(){
    
    const static GLfloat ground[][4] = {
        { 0.6, 0.6, 0.6, 1.0 },
        { 0.3, 0.3, 0.3, 1.0 }
    };
    int i, j;   
    glBegin(GL_QUADS);
    glNormal3d(0.0, 0.0, 1.0);
    for (j = -100 / 2; j < 100 / 2; ++j) {
        for (i = -100 / 2; i < 100 / 2; ++i) {
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, ground[(i + j) & 1]);
            glVertex3d((GLdouble)i, (GLdouble)j, 0);
            glVertex3d((GLdouble)i, (GLdouble)(j + 1), 0);
            glVertex3d((GLdouble)(i + 1), (GLdouble)(j + 1), 0);
            glVertex3d((GLdouble)(i + 1), (GLdouble)j, 0);
      }
    }
    glEnd();
}

//ゴミ箱
void trash_can() {

    float c1[4] = { 1.0f, 0.36f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c1);
  
    //側面
    glBegin(GL_QUAD_STRIP);
    for(double i=0;i<=100;i=i+1){
        double t = i*2*pi/100;
        if(mode == 1) {
            //ゴミ箱固定モード
            glNormal3d((GLfloat)cos(t),(GLfloat)sin(t),0.0);
            glVertex3f((GLfloat)(5*cos(t)),(GLfloat)(5*sin(t)) + 30,10);
            glVertex3f((GLfloat)(3*cos(t)) ,(GLfloat)(3*sin(t)) + 30,0);
        }else if(mode == 2){
            //ゴミ箱移動モード
            glNormal3d((GLfloat)cos(t) + sin(TIME/32),(GLfloat)sin(t),0.0);
            glVertex3f((GLfloat)(5*cos(t)) + 20 * sin(TIME/32),(GLfloat)(5*sin(t)) + 30,10);
            glVertex3f((GLfloat)(3*cos(t)) + 20 * sin(TIME/32),(GLfloat)(3*sin(t)) + 30,0);
        }
    }
    glEnd();

    //下面
    glNormal3d(0.0, 0.0, 1.0);
    glBegin(GL_POLYGON);
    for(double i = 100; i >= 0; --i) {
        double t = pi*2/100 * (double)i;
        if(mode == 1) {
            //ゴミ箱固定モード
            glVertex3d(3 * cos(t), 3 * sin(t) + 30, 0);
        }else if(mode == 2) {
            //ゴミ箱移動モード
            glVertex3d(3 * cos(t) + 20 * sin(TIME/32), 3 * sin(t) + 30, 0);
        }
    }
    glEnd();
}
//発射位置のゴミ
void trash() {
    float c[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c);
    glPushMatrix();
    glTranslated(xc, -10, 3.0);
    glutSolidSphere(3.0, 64, 32);
    glPopMatrix();    
}

//--------------------------------------------------------
// ルンゲクッタ法
//--------------------------------------------------------
void f(double w1, double w2, double g, matrix k0, matrix *k1) {
    matrix k2;
    mat_alloc(&k2,2,1);
    mat_elem(k2,0,0) = mat_elem(k0,1,0);
    mat_elem(k2,1,0) = -w1*mat_elem(k0,0,0) - g -w2*mat_elem(k0,1,0);
    mat_copy(k1,k2);
}
void runge_kutta(double w1, double w2, double g, matrix y0, double h, matrix *y1, void (*f)(double, double, double, matrix, matrix *)) {

    matrix k1,k2,k3,k4; 
    mat_alloc(&k1,2,1);
    mat_elem(k1,0,0) = 0.0;
    mat_elem(k1,1,0) = 0.0;
    mat_alloc(&k2,2,1);
    mat_elem(k2,0,0) = 0.0;
    mat_elem(k2,1,0) = 0.0;
    mat_alloc(&k3,2,1);
    mat_elem(k3,0,0) = 0.0;
    mat_elem(k3,1,0) = 0.0;
    mat_alloc(&k4,2,1);
    mat_elem(k4,0,0) = 0.0;
    mat_elem(k4,1,0) = 0.0;
    //k1
    f(w1,w2,g,y0,&k1);
    //k2
    mat_muls(&k2,k1,h/2);
    mat_add(&k2,y0,k2);
    f(w1,w2,g,k2,&k2);
    //k3
    mat_muls(&k3,k2,h/2);
    mat_add(&k3,y0,k3);
    f(w1,w2,g,k3,&k3);
    //k4
    mat_muls(&k4,k3,h);
    mat_add(&k4,y0,k4);
    f(w1,w2,g,k4,&k4);  
    mat_muls(&k2,k2,2);
    mat_add(&k1,k1,k2);
    mat_muls(&k3,k3,2);
    mat_add(&k1,k1,k3);
    mat_add(&k1,k1,k4);
    mat_muls(&k1,k1,h/6);
    mat_add(&y0,y0,k1);
    mat_copy(y1,y0);
}
//--------------------------------------------------------

//ゴミを投げる
void throw_trash(double w) {
    if(shoot == 1){
        float c[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c);
        
        //ゴミの描く軌道を計算
        //空気抵抗の比例定数0.25
        //風力ws
        runge_kutta(0.0, 0.25, w, x_0, 0.5, &x, f);
        double xt = mat_elem(x,0,0)/20.0;
        runge_kutta(0.0, 0.25, 0.0, y_0, 0.5, &y, f);
        double yt = mat_elem(y,0,0)/20.0 - 10.0;
        runge_kutta(0.0, 0.25, 9.8, z_0, 0.5, &z, f);
        double zt = mat_elem(z,0,0)/20.0 + 3.0;
    
        glPushMatrix();
        glTranslated(xt, yt, zt);
        glutSolidSphere(3.0, 64, 32);
        
        glPopMatrix();
        if(mode == 1){
           if(fabs(xt) <= 2.0 && fabs(yt - 30) <= 2.0 && zt <= 10 && zt >= 9){
                shoot = 0; //発射状態のゴミがなくなる
                points++; //連続で入ったとき得点加算
                if(high_score < points) high_score = points; //最高得点の更新
            }else if (fabs(xt) <= 8.0 && fabs(yt - 30) <= 8.0 && zt <= 10 && zt >= 9){
                //ゴミがゴミ箱の縁に当たったとき
                shoot = 0;
                points = 0; //はずしたら得点リセット
            }else if (zt < 3){
                //ゴミが地面についたとき
                shoot = 0;
                points = 0; 
            } 
        }
        if(mode == 2){
            if(fabs(xt - 20 * sin(TIME/32)) <= 2.0 && fabs(yt - 30) <= 2.0 && zt <= 10 && zt >= 9){
                shoot = 0;
                points++;
            }else if (fabs(xt - 20 * sin(TIME/32)) <= 8.0 && fabs(yt - 30) <= 8.0 && zt <= 10 && zt >= 9){
                shoot = 0;
                points = 0;
            }else if (zt < 3){
                shoot = 0;
                points = 0;
            }
        }
    }
}

// 描画関数
void display() {
    // 描画内容のクリア
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // マウスによる変形の設定
    glPushMatrix();
    glScaled(SCALE, SCALE, SCALE);

    // --- この下を変更すると良い ----------------------------
    ground();
    trash_can();
    trash();
    DRAW_POINTS(points);
    DRAW_HIGHSCORE(high_score);
    DRAW_WIND(w2); //次投げるときの風速表示
    throw_trash(w1);
    // --- この上を変更すると良い ----------------------------

    // マウスによる変形の破棄
    glPopMatrix();

    // 描画命令
    glutSwapBuffers();
}

// ウィンドウサイズ変更時の処理関数
void reshape(int width, int height) {
    // ビューポートの設定
    glViewport(0, 0, width, height);
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;

    // 投影変換行列の設定
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const double aspect = (double)WINDOW_WIDTH / (double)WINDOW_HEIGHT;
    gluPerspective(45.0, aspect, 0.1, 100.0);

    // モデルビュー行列の設定
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(CAMERA_POS[0], CAMERA_POS[1], CAMERA_POS[2],   // カメラの位置
              0.0, 0.0, 10.0,   // カメラが見ている位置の中心
              0.0, 1.0, 0.0);  // カメラの上方向
}

// 発射位置の移動
void mouse(int x, int y) {
    xc = (x-WINDOW_WIDTH/2) / 20; 
}

// キーボードが押された時の処理
void keyboard(unsigned char key, int xk, int yk) {
    if(key == ' ' && shoot == 0) {
        //発射状態のゴミがないとき発射
        mat_alloc(&x_0,2,1);
        mat_elem(x_0,0,0) = xk-WINDOW_WIDTH/2;
        mat_elem(x_0,1,0) = 0.0;
        mat_alloc(&x,2,1);
        mat_elem(x,0,0) = 0.0;
        mat_elem(x,1,0) = 0.0;
        mat_alloc(&y_0,2,1);
        mat_elem(y_0,0,0) = 0.0;
        mat_elem(y_0,1,0) = 200.0;
        mat_alloc(&y,2,1);
        mat_elem(y,0,0) = -10.0;
        mat_elem(y,1,0) = 0.0;
        mat_alloc(&z_0,2,1);
        mat_elem(z_0,0,0) = 0.0;
        mat_elem(z_0,1,0) = 180.0;
        mat_alloc(&z,2,1);
        mat_elem(z,0,0) = 0.0;
        mat_elem(z,1,0) = 0.0;
        shoot = 1;
        w1 = w2; //現在の風速を更新
        w2 = (double)(rand()%500)/50.0 - 5.0; //次投げるときの風速 
    } else if(key == '1') {
        mode = 1; //モードの切り替え
    } else if(key == '2') {
        mode = 2;
    } else if(key == 'r') {
        high_score = 0; //最高得点のリセット
    } else if (key == '\e') {
        exit(1);
    }
}

// アニメーションの処理
void timer(int value) {
    TIME += 1.0;
    glutPostRedisplay();
    glutTimerFunc(30, timer, 0);
}

// メイン関数
int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("GLUT 3D graphics");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(30, timer, 0);

    init();

    glutMainLoop();
}