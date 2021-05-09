#include <stdio.h>
#include <GLUT/glut.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define H_WIN 400                      // ウィンドウの幅
#define W_WIN 300                      // ウィンドウの高さ

#define W2_HODAI 10                    // 砲台の横幅の半分
#define H_HODAI 15                     // 砲台の上面のy座標
#define L_HODAI 5                      // 砲台の下面のy座標
#define L_E_BEAM 20                    // 防衛軍のビームの長さ
#define V_E_BEAM 1.5                   // 防衛軍のビームの速度
#define N_E_BEAM 1                     // 防衛軍のビームの画面上の最大数

#define L_I_BEAM 10                    // インベーダー軍のビームの長さ
#define V_I_BEAM 0.8                   // インベーダー軍のビームの速度
#define P_I_BEAM 500                   // インベーダー軍のビームの初期発射確率

#define N_I_BEAM 20                    // インベーダー軍のビームの画面上の最大数
#define NXIV 9                         // インベーダー軍の列の数
#define NYIV 4                         // インベーダー軍の行の数
#define V_INVADER 0.1                  // インベーダー軍の速度

#define NOT_DECIDE 0
#define INVADER 1
#define HUMAN 2

//---- プロトタイプ宣言 -----
void initialize(void);                      // 初期化

void draw(void);                            // 図を描く
void draw_result(void);                     // 結果表示
void draw_hodai(void);                      // 防衛軍の砲台の描画
void draw_e_beam(void);                     // 防衛軍のビームの描画
void draw_i_beam(void);                     // インベーダー軍のビームの描画
void draw_invader(void);                    // インベーダー軍の描画

void change_state(void);                    // 状態変化に関する処理
void state_e_beam(void);                    // 防衛軍のビームの状態変化
void state_invader(void);                   // インベーダー軍の状態変化
void state_i_beam(void);                    // インベーダー軍のビーム状態変化

void mouse_xy(int x, int y);
void shoot(unsigned char key, int x, int y); // 防衛軍ビーム発射

void resize(int w, int h);                  // サイズの調整
void set_color(void);                       // 塗りつぶし色の設定

//---- グローバル変数 -------
double xc = 100.0;                          // マウスのx座標

typedef struct{
  unsigned char status;                     // 0:dead 1:alive
  double x, y;                              // 中心座標
}invader;

invader invd[NXIV][NYIV];                   // インベーダー
int alive_inv=NXIV*NYIV;                    // 生きているインベーダーの数
double inv_vx=V_INVADER;                    // インベーダーの横方向の速度


typedef struct{
  char status;                              // 0:格納庫 1:砲台の上 2:移動中
  double x;                                 // ビームのx座標
  double y0, y1;                            // ビームのy座標 y0:先頭 y1:最後尾
  double vy;                                // ビームの速度
}beam;

beam e_beam[N_E_BEAM];                      // 地球防衛軍のビーム
beam *p_e_beam1;                            // 地球防衛軍の次に発射可能なビーム
beam i_beam[N_I_BEAM];                      // インベーダー軍のビーム

int winner = NOT_DECIDE;
char *win="You won a game.";
char *lost="You lost a game.";
char *push_key="push a key";
char *quit_new="n:new game      q:quit";

//====================================================================
// main関数
//====================================================================
int main(int argc, char *argv[])
{

  initialize();
  glutInitWindowPosition(100,200);          // 初期位置(x,y)指定
  glutInitWindowSize(W_WIN,H_WIN);          // 初期サイズ(幅，高さ)指定
  glutInit(&argc, argv);                    // GLUT 初期化
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);   // 表示モードの指定
  glutCreateWindow("space invader modoki");       // windowをタイトルを付けてを開く
  glutDisplayFunc(draw);                    // イベントにより呼び出し
  glutReshapeFunc(resize);                  // サイズ変更のときに呼び出す関数指定
  glutIdleFunc(change_state);               // 暇なときに実行(状態の変化)
  glutPassiveMotionFunc(mouse_xy);          // マウスイベント(砲台の移動)
  glutKeyboardFunc(shoot);                  // キーボードイベント(ビームを発射)
  set_color();                              // 塗りつぶす色指定
  glutMainLoop();                           // GLUTの無限ループ

  return 0;
}

//====================================================================
// 初期化
//====================================================================
void initialize(void)
{
  int i, j;

  srand((unsigned int)time(NULL));        // 乱数を発生させるため

  for(i=0; i<N_E_BEAM; i++){
    e_beam[i].status=0;
    e_beam[i].y0=H_HODAI+L_E_BEAM;
    e_beam[i].y1=H_HODAI;
    e_beam[i].vy=0.0;
  }

  e_beam[0].status=1;                       // 砲台にのせる
  p_e_beam1=&e_beam[0];

  for(i=0; i<N_I_BEAM; i++){
    i_beam[i].status = 0;
    i_beam[i].y0 = 0;
    i_beam[i].y1 = 0;
    i_beam[i].vy = V_I_BEAM;
  }

  for(i=0; i<NXIV; i++){
    for(j=0; j<NYIV; j++){
      invd[i][j].status=1;
      invd[i][j].x = 20*(i+1);            // x,yとも20ピクセル間隔
      invd[i][j].y = H_WIN - NYIV*20+10+20*j;
    }
  }
}


//====================================================================
// 図を描く
//====================================================================
void draw(void)
{
  glClear(GL_COLOR_BUFFER_BIT);

  if(winner != NOT_DECIDE) draw_result();

  draw_hodai();         // 砲台を描く関数呼び出し
  draw_e_beam();        // 地球防衛軍のビームを描く関数の呼び出し
  draw_i_beam();        // インベーダー軍のビームを描く関数の呼び出し
  draw_invader();       // インベーダーを描く関数の呼び出し
  
  glutSwapBuffers();    // 描画
}


//====================================================================
// 勝者の表示
//====================================================================
void draw_result(void)
{
    int i=0;

    glColor3d(0.0, 1.0, 0.0);

    if(winner==HUMAN){
      while(win[i]!='\0'){
	glRasterPos2i(50+15*i,100);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,win[i]);
	i++;
      }
    }else if(winner==INVADER){
      while(lost[i]!='\0'){
	glRasterPos2i(50+15*i,100);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,lost[i]);
	i++;
      }
    }

    // ゲームリスタート or 終了 に関するメッセージ表示
    glColor3d(1.0, 0.0, 0.0);
    glRasterPos2i(30,160);
    for(i=0; push_key[i]!='\0'; i++){
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,push_key[i]);
    }
    glColor3d(0.7, 0.0, 0.0);
    glRasterPos2i(40,140);
    for(i=0; quit_new[i]!='\0'; i++){
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,quit_new[i]);
    }
    
}


//====================================================================
// 地球防衛軍の砲台の描画
//====================================================================
void draw_hodai(void)
{
  glColor3d(0.5, 1.0, 0.5);            // 線の色指定(RGB)
  glBegin(GL_POLYGON);
  glVertex2d(xc-W2_HODAI, L_HODAI);
  glVertex2d(xc+W2_HODAI, L_HODAI);
  glVertex2d(xc+W2_HODAI, H_HODAI);
  glVertex2d(xc-W2_HODAI, H_HODAI);
  glEnd();
}


//====================================================================
// 地球防衛軍のビーム砲の描画
//====================================================================
void draw_e_beam(void)
{
  int i;

  for(i=0;i<N_E_BEAM;i++){
    if(e_beam[i].status != 0){
      glColor3d(1.0, 0.0, 0.0);            // 線の色指定(RGB)
      glBegin(GL_LINES);
      glVertex2d(e_beam[i].x, e_beam[i].y0);
      glVertex2d(e_beam[i].x, e_beam[i].y1);
      glEnd();
    }
  }
}


//====================================================================
// インベーダー軍のビームの描画
//====================================================================
void draw_i_beam(void)
{
  int i;

  for(i=0; i<N_I_BEAM; i++){
    if(i_beam[i].status == 2){
      glColor3d(0.0, 0.0, 1.0);            // 線の色指定(RGB)
      glBegin(GL_LINES);
      glVertex2d(i_beam[i].x, i_beam[i].y0);
      glVertex2d(i_beam[i].x, i_beam[i].y1);
      glEnd();
    }
  }
}


//====================================================================
// インベーダー軍の描画
//====================================================================
void draw_invader(void)
{
  int i, j;     // インベーダーのi列j行

  for(i=0; i<NXIV; i++){
    for(j=0; j<NYIV; j++){
      if(invd[i][j].status==1){    // 生きているインベーダーのみを描く

	//------ 胴体 ----------------------
	glColor3d(1.0, 1.0, 1.0);
	glBegin(GL_POLYGON);
	glVertex2d(invd[i][j].x-8, invd[i][j].y);
	glVertex2d(invd[i][j].x-3, invd[i][j].y-4);
	glVertex2d(invd[i][j].x+3, invd[i][j].y-4);
	glVertex2d(invd[i][j].x+8, invd[i][j].y);
	glVertex2d(invd[i][j].x+3, invd[i][j].y+4);
	glVertex2d(invd[i][j].x-3, invd[i][j].y+4);
	glEnd();

	//------- 手 足 触覚 -----------------------
	glBegin(GL_LINES);
	glVertex2d(invd[i][j].x-7, invd[i][j].y);    // 左手
	glVertex2d(invd[i][j].x-7, invd[i][j].y+6);
	glVertex2d(invd[i][j].x+7, invd[i][j].y);    // 右手
	glVertex2d(invd[i][j].x+7, invd[i][j].y+6);
	glVertex2d(invd[i][j].x-4, invd[i][j].y-4);  // 左足
	glVertex2d(invd[i][j].x-6, invd[i][j].y-8);
	glVertex2d(invd[i][j].x+4, invd[i][j].y-4);  // 右足
	glVertex2d(invd[i][j].x+6, invd[i][j].y-8);
	glVertex2d(invd[i][j].x-2, invd[i][j].y+4);  // 左触覚
	glVertex2d(invd[i][j].x-5, invd[i][j].y+6);
	glVertex2d(invd[i][j].x+2, invd[i][j].y+4);  // 右触覚
	glVertex2d(invd[i][j].x+5, invd[i][j].y+6);
	glEnd();

	//------- 目玉 ----------------
	glColor3d(0.0, 0.0, 0.0);
	glBegin(GL_POLYGON);             // 左目
	glVertex2d(invd[i][j].x-3, invd[i][j].y);
	glVertex2d(invd[i][j].x-1, invd[i][j].y);
	glVertex2d(invd[i][j].x-1, invd[i][j].y+2);
	glVertex2d(invd[i][j].x-3, invd[i][j].y+2);
	glEnd();
	glBegin(GL_POLYGON);             // 右目
	glVertex2d(invd[i][j].x+3, invd[i][j].y);
	glVertex2d(invd[i][j].x+1, invd[i][j].y);
	glVertex2d(invd[i][j].x+1, invd[i][j].y+2);
	glVertex2d(invd[i][j].x+3, invd[i][j].y+2);
	glEnd();
      }
    }
  }
}


//====================================================================
// リサイズ
//     この関数は window のサイズが変化したら呼び出される
//     引数
//            w:ウィンドウの幅
//            h:ウィンドウの高さ
//====================================================================
void resize(int w, int h)
{
  glLoadIdentity();                   // 変換行列を単位行列に
  gluOrtho2D(0, W_WIN, 0, H_WIN);     // world座標系の範囲
  glViewport(0, 0, w, h);             // ウィンドウ座標系を指定
}


//====================================================================
// PCが暇なときに実行する．これが実行されると状態が変化する
//====================================================================
void change_state(void)
{

  if(winner == NOT_DECIDE){
    state_e_beam();     // 地球防衛軍のビームの処理
    state_invader();    // インベーダー軍の処理
    state_i_beam();     // インベーダー軍のビームの処理
  }

  glutPostRedisplay();
}


//====================================================================
// 地球防衛軍のビームの状態の処理
//====================================================================
void state_e_beam(void)
{
  int i,l,m;
  int st0=0;
  int rdy=0;
  int nshoot=0;                 // 発射済みの地球防衛軍の玉の数
  double min_y=H_WIN+L_E_BEAM;   // 最もしたのミサイルの先のy座標
  double ydis;                  // 最も下のミサイルと発射台の距離

  for(i=0; i<N_E_BEAM; i++){
    switch(e_beam[i].status){

    //--------  格納庫にあるビームの処理 ------------------------
    case 0:
      st0=i;                    // 次に発射可能なビームを設定
      break;

    //--------  砲台にあるビームの処理 ------------------------
    case 1:
      e_beam[i].x = xc;         // x方向に移動
      rdy=1;                    // 砲台にビームがあることを示すフラグをON
      break;

    //--------  すでに発射されたビームの処理 ------------------------
    case 2:
      nshoot++;                         // 発射されているビームをカウント
      e_beam[i].y0 += e_beam[i].vy;     // ビームの移動
      e_beam[i].y1 += e_beam[i].vy;

      // ------ インベーダーにビームが衝突したことを確認して処理 ------
      for(l=0; l<NXIV; l++){    
	for(m=0; m<NYIV; m++){
	  if(invd[l][m].status==1){
	    if((invd[l][m].x-8 < e_beam[i].x) &&
	       (e_beam[i].x < invd[l][m].x+8) &&
	       (invd[l][m].y-4 < e_beam[i].y0) &&
	       (e_beam[i].y1 < invd[l][m].y+4)){
	      invd[l][m].status=0;            // インベーダーの死亡
	      alive_inv--;                    // 生きているインベーダーの数を-1
	      if(alive_inv==0)winner=HUMAN;
	      e_beam[i].status=0;             // ビームは格納庫へ
	      e_beam[i].y0=H_HODAI+L_E_BEAM;  // ビームの初期化
	      e_beam[i].y1=H_HODAI;
	    }
	  }
	}      
      }


      // ---- 画面から地球防衛軍のビームがはみ出た場合の処理 --------
      if(H_WIN+L_E_BEAM < e_beam[i].y0){
	e_beam[i].status = 0;
	e_beam[i].y0 = H_HODAI+L_E_BEAM;
	e_beam[i].y1 = H_HODAI;
	e_beam[i].vy = 0.0;
      }
      if(e_beam[i].y0 < min_y) min_y=e_beam[i].y0;
      break;
    default:
      printf("e_beam status error!!\n");
      exit(1);
    }
  }


  // --- 地球防衛軍の新たな発射可能なビームの処理 -----
  ydis = min_y-H_HODAI;
  if( (2.5*L_E_BEAM < ydis) && (rdy==0) && (nshoot<N_E_BEAM) ){
    e_beam[st0].status=1;
    p_e_beam1=(beam *)&e_beam[st0];     // 発射可能なビームをポインターで表現
  }
}


//====================================================================
// インベーダー軍の状態の処理
//====================================================================
void state_invader(void)
{
  int i, j, k;
  double ivmin_x=W_WIN, ivmax_x=0;
  double ivmin_y=H_WIN, ivmax_y=0;
  int can_attack;

  for(i=0; i<NXIV; i++){
    can_attack=1;
    for(j=0; j<NYIV; j++){
      if(invd[i][j].status==1){   // インベーダーの生死のチェック
	invd[i][j].x += inv_vx;   // インベーダーの横方向移動
	// ---- インベーダー軍のビーム発射の処理 ------
	if(can_attack == 1 && rand()%P_I_BEAM == 0){  // 発射条件
	  for(k=0; k<N_I_BEAM; k++){
	    if(i_beam[k].status !=2){      // 発射可能なビームを探す
	      i_beam[k].status =2;         // ビームの発射
	      i_beam[k].x = invd[i][j].x;
	      i_beam[k].y0 = invd[i][j].y;
	      i_beam[k].y1 = invd[i][j].y-L_I_BEAM;
	      break;
	    }
	  }
	}
	// --- インベーダー軍の左右上下の端の座標 -------
	if(invd[i][j].x < ivmin_x) ivmin_x=invd[i][j].x;   // 左端 
	if(invd[i][j].x > ivmax_x) ivmax_x=invd[i][j].x;   // 右端
	if(invd[i][j].y < ivmin_y) ivmin_y=invd[i][j].y;   // 下の端
	if(invd[i][j].y > ivmax_y) ivmax_y=invd[i][j].y;   // 上の端
	can_attack=0;
      }
    }
  }


  if(ivmin_x < 10) inv_vx = V_INVADER;           // 左端に達したとき
  if(ivmax_x > W_WIN-10) inv_vx = -V_INVADER;    // 右端に達したとき
  
  if((ivmin_x < 10) || (ivmax_x > W_WIN-10)){    // 左右の端に達しとき
    for(i=0; i<NXIV; i++){
      for(j=0; j<NYIV; j++){
	invd[i][j].y -= 10;                       // 下に降りる
      }
    }
  }
}


//====================================================================
// インベーダー軍のビームの状態の処理
//====================================================================
void state_i_beam(void)
{
  int i;

  for(i=0; i<N_I_BEAM; i++){
    if(i_beam[i].status ==2){
      i_beam[i].y0 -= i_beam[i].vy;
      i_beam[i].y1 -= i_beam[i].vy;
   
      if(i_beam[i].y1 < 0) i_beam[i].status=0;

      if((xc-W2_HODAI < i_beam[i].x) &&
	 (i_beam[i].x < xc+W2_HODAI) &&
	 (L_HODAI < i_beam[i].y0) &&
	 (i_beam[i].y1 < H_HODAI)){
	winner=INVADER;            // 地球防衛軍の負け
      }
    }
  }
}


//====================================================================
// マウスイベントの処理
//====================================================================
void mouse_xy(int x, int y)
{
  xc=x;                // マウスのx座標をグローバル変数の xc へ代入
}


//====================================================================
// キーボードイベントの処理
// スペースキーが押されたら地球防衛軍のビームを発射
//====================================================================
void shoot(unsigned char key, int x, int y)
{
  //--- スペースキーが押されて，発射可能なビームがあるとき ----
  if(key==' ' && p_e_beam1 != NULL){
    p_e_beam1->status = 2;            // ビームを発射の状態にする
    p_e_beam1->vy = V_E_BEAM;         // ビームの速度を設定
    p_e_beam1 = NULL;                 // 発射可能なビームが無い
  }

  //----- game 終了とリスタートの選択 -----
  if(winner != NOT_DECIDE){
    if(key=='q'){
      exit(0);
    }else if(key=='n'){
      initialize();                   // 初期化
      winner = NOT_DECIDE;            // 勝者は未定に
      alive_inv=NXIV*NYIV;            // 生きているインベーダーの数
    }
  }

}


//====================================================================
// 色の指定
//====================================================================
void set_color(void)
{
  glClearColor(0.0, 0.0, 0.0, 1.0);        //赤緑青と透明度
}