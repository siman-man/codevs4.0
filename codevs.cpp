#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <limits.h>
#include <string>
#include <string.h>
#include <sstream>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <stack>
#include <queue>

using namespace std;

typedef long long ll;

const int WORKER  = 0; // ワーカー
const int KNIGHT  = 1; // ナイト
const int FIGHER  = 2; // ファイター
const int ASSASIN = 3; // アサシン
const int CASTEL  = 4; // 城
const int VILLAGE = 5; // 村
const int BASE    = 6; // 拠点

const int UNIT_MAX = 7; // ユニットの種類

// 各ユニットの生産にかかるコスト
const int unitCost[UNIT_MAX] = {40, 20, 40, 60, -1, 100, 500};
// 各ユニットのHP
const int unitHp[UNIT_MAX] = {2000, 5000, 5000, 5000, 50000, 20000, 20000};
// 各ユニットの攻撃範囲
const int unitAttackRange[UNIT_MAX] = {2, 2, 2, 2, 10, 2, 2};
// 各ユニットの視野
const int unitEyeRange[UNIT_MAX] = { 4, 4, 4, 4, 10, 10, 4};

const int MAX_UNIT_ID   = 20010;  // ユニットのIDの上限
const int HEIGHT        = 100;    // フィールドの横幅
const int WIDTH         = 100;    // フィールドの縦幅

int absDist[WIDTH*WIDTH];

// プレイヤーの名前
const string PLAYER_NAME = "siman";

// ダメージテーブル
const int DAMAGE_TABLE[7][7] = {
  /*          労   騎   闘   殺   城   村   拠 */
  /* 労 */ { 100, 100, 100, 100, 100, 100, 100}, 
  /* 騎 */ { 100, 500, 200, 200, 200, 200, 200}, 
  /* 闘 */ { 500,1600, 500, 200, 200, 200, 200},
  /* 殺 */ {1000, 500,1000, 500, 200, 200, 200}, 
  /* 城 */ { 100, 100, 100, 100, 100, 100, 100}, 
  /* 村 */ { 100, 100, 100, 100, 100, 100, 100},
  /* 拠 */ { 100, 100, 100, 100, 100, 100, 100}
};

// ユニットが持つ構造
struct Unit{
  int id;           // ユニットの種別を表すID
  int hp;           // HP
  int eyeRange;     // 視野
  int attackRange;  // 攻撃範囲
  bool canMove;     // 移動できるかどうか
};

// フィールドの1マスに対応する
struct Node{
  int id;           // ノードのID
  int y;            // y座標
  int x;            // x座標
};

int remainingTime;            // 残り時間
int stageNumber;              // 現在のステージ数
int currentStageNumber;       // 現在のステージ数
int turn;                     // 現在のターン
int myUnitCount;              // 自軍のユニット数
int enemyUnitCount;           // 敵軍のユニット数
int resourceCount;            // 資源の数
int myResourceCount;          // 自軍の資源の数
Unit* unitList[MAX_UNIT_ID];  // ユニットのリスト

char field[HEIGHT][WIDTH];    // ゲームフィールド


class Codevs{
  public:
    /*
     * ゲームの初期化処理
     */
    void init(){
      currentStageNumber = -1;
      memset(field, -1, sizeof(field));

      absDistInitialize();

      // 一番最初でプレイヤー名の出力
      printf("%s\n", PLAYER_NAME.c_str());
    }

    /*
     * マンハッタン距離の初期化
     */
    void absDistInitialize(){
      string str;
      ifstream ifs("absDist.txt");

      if(ifs.fail()){
        fprintf(stderr, "Failed\n");
      }   

      int i = 0;
      int dist;
      while(getline(ifs, str)){
        dist = atoi(str.c_str());
        absDist[i] = dist;
        i++;
      }   
    }

    /*
     * ステージの初期化
     */
    void stageInitialize(){
    }

    /*
     * 各ターンの入力処理
     */
    void eachTurnProc(){
      int unitId;   // ユニットID
      int y;        // y座標
      int x;        // x座標
      int hp;       // HP
      int unitType; // ユニットの種類
      string str;   // 終端文字列「END」を格納するだけの変数

      // 残り時間(ms)
      scanf("%d", &remainingTime);

      // 現在のステージ数(0-index)
      scanf("%d", &stageNumber);

      if(stageNumber != currentStageNumber){
        stageInitialize();
        currentStageNumber = stageNumber;
      }

      // 現在のターン数(0-index)
      scanf("%d", &turn);

      // 資源数
      scanf("%d", &myResourceCount);

      // 自軍のユニット数
      scanf("%d", &myUnitCount);

      // 自軍ユニットの詳細
      for(int i = 0; i < myUnitCount; i++){
        scanf("%d %d %d %d %d", &unitId, &y, &x, &hp, &unitType);
      }

      // 視野内の敵軍のユニット数
      scanf("%d", &enemyUnitCount);

      for(int i = 0; i < enemyUnitCount; i++){
        scanf("%d %d %d %d %d", &unitId, &y, &x, &hp, &unitType);
      }

      // 視野内の資源の数
      scanf("%d", &resourceCount);

      for(int i = 0; i < resourceCount; i++){
        scanf("%d %d", &y, &x);
      }

      // 終端文字列
      cin >> str;
    }

    void run(){
      init();
    }

    /*
     * 指示フェーズ
     */
    void instruction(){
    }

    /*
     * 行動フェーズ
     */
    void move(){
    }

    /*
     * 戦闘フェーズ
     */
    void battle(){
    }

    /*
     * 除外フェーズ
     */
    void exclude(){
    }

    /*
     * 資源獲得フェーズ
     */
    void obtain(){
    }

    /*
     * 渡された座標の距離を計算
     */
    int calcDist(int y1, int x1, int y2, int x2){
      return absDist[x1*WIDTH+x2] + absDist[y1*HEIGHT+y2];
    }

    /*
     * フィールドの表示
     */
    void showField(){
      for(int y = 0; y < HEIGHT; y++){
        for(int x = 0; x < WIDTH; x++){
          fprintf(stderr, "%02d", field[y][x]);
        }
        fprintf(stderr, "\n");
      }
    }
};

class CodevsTest{
  Codevs cv;

  public:
  void testRun(){
    fprintf(stderr, "TestCase1: %s\n", testCase1()? "SUCCESS!" : "FAILED!");
  }

  bool testCase1(){
    if(cv.calcDist(0,0,1,1) != 2) return false;
    if(cv.calcDist(0,0,0,0) != 0) return false;
    if(cv.calcDist(99,99,99,99) != 0) return false;
    if(cv.calcDist(0,99,99,0) != 198) return false;
    if(cv.calcDist(3,20,9,19) != 7) return false;

    return true;
  }
};

int main(){
  Codevs cv;
  CodevsTest cvt;

  cv.run();
  cvt.testRun();

  return 0;
}
