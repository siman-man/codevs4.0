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

// ユニット一覧
const int WORKER  = 0; // ワーカー
const int KNIGHT  = 1; // ナイト
const int FIGHER  = 2; // ファイター
const int ASSASIN = 3; // アサシン
const int CASTEL  = 4; // 城
const int VILLAGE = 5; // 村
const int BASE    = 6; // 拠点

// 行動一覧
const int NONE            =  0; // 何も移動しない
const int MOVE_UP         =  1; // 上に移動
const int MOVE_DOWN       =  2; // 下に移動
const int MOVE_LEFT       =  3; // 左に移動
const int MOVE_RIGHT      =  4; // 右に移動
const int CREATE_WORKER   =  5; // ワーカーを生産
const int CREATE_KNIGHT   =  6; // ナイトを生産
const int CREATE_FIGHER   =  7; // ファイターを生産
const int CREATE_ASSASIN  =  8; // アサシンを生産
const int CREATE_VILLAGE  =  9; // 村を生産
const int CREATE_BASE     = 10; // 拠点

const int UNIT_MAX = 7; // ユニットの種類

// 各ユニットの生産にかかるコスト
const int unitCost[UNIT_MAX] = {40, 20, 40, 60, -1, 100, 500};
// 各ユニットのHP
const int unitHp[UNIT_MAX] = {2000, 5000, 5000, 5000, 50000, 20000, 20000};
// 各ユニットの攻撃範囲
const int unitAttackRange[UNIT_MAX] = {2, 2, 2, 2, 10, 2, 2};
// 各ユニットの視野
const int unitEyeRange[UNIT_MAX] = { 4, 4, 4, 4, 10, 10, 4};
// 各ユニットの行動の可否
const int unitCanMove[UNIT_MAX] = {true, true, true, true, false, false, false};

const int MAX_UNIT_ID   = 20010;  // ユニットのIDの上限
const int HEIGHT        = 100;    // フィールドの横幅
const int WIDTH         = 100;    // フィールドの縦幅

int absDist[WIDTH*WIDTH];

// プレイヤーの名前
const string PLAYER_NAME = "siman";

// ダメージテーブル [攻撃する側][攻撃される側]
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

// 各ユニットが出来る行動 [ユニットID][行動リスト]
const bool OPERATION_LIST[7][10] = {
  /*         動上   動下   動左   動右   産労   産騎   産闘   産殺   産村   産拠 */
  /* 労 */ { true,  true,  true,  true, false, false, false, false,  true,  true},
  /* 騎 */ { true,  true,  true,  true, false, false, false, false, false, false},
  /* 闘 */ { true,  true,  true,  true, false, false, false, false, false, false},
  /* 殺 */ { true,  true,  true,  true, false, false, false, false, false, false},
  /* 城 */ {false, false, false, false,  true, false, false, false, false, false},
  /* 村 */ {false, false, false, false,  true, false, false, false, false, false},
  /* 拠 */ {false, false, false, false, false,  true,  true,  true, false, false}
};

// ユニットが持つ構造
struct Unit{
  int id;           // ユニットのID
  int y;            // y座標
  int x;            // x座標
  int hp;           // HP
  int type;         // ユニットの種別
  int eyeRange;     // 視野
  int attackRange;  // 攻撃範囲
  bool canMove;     // 移動できるかどうか
};

// フィールドの1マスに対応する
struct Node{
  bool opened;        // 一度調査したことがあるかどうか
};

// ゲーム・フィールド全体の構造
struct GameField{
  short openNodeCount;
};

int remainingTime;            // 残り時間
int stageNumber;              // 現在のステージ数
int currentStageNumber;       // 現在のステージ数
int turn;                     // 現在のターン
int myUnitCount;              // 自軍のユニット数
int enemyUnitCount;           // 敵軍のユニット数
int resourceCount;            // 資源の数
int myResourceCount;          // 自軍の資源の数
Unit unitList[MAX_UNIT_ID];  // ユニットのリスト

Node field[HEIGHT][WIDTH];         // ゲームフィールド
Node tempField[HEIGHT][WIDTH];    // 一時的なゲームフィールド
map<int, bool> unitIdCheckList;     // IDが存在しているかどうかのチェック


class Codevs{
  public:
    /*
     * ゲームの初期化処理
     */
    void init(){
      currentStageNumber = -1;
      memcpy(tempField, field, sizeof(field));

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
     * ステージ開始直前に行う初期化処理
     */
    void stageInitialize(){
      unitIdCheckList.clear();

      for(int y = 0; y < HEIGHT; y++){
        for(int x = 0; x < WIDTH; x++){
          Node node;

          field[y][x] = node;
        }
      }
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

      // 現在のステージ数(0-index)
      scanf("%d", &stageNumber);

      /* 
       * 今のステージ数と取得したステージ数が異なる場合は
       * 新規ステージなので初期化を行う
       */
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

        // チェックリストに載っていない場合は、新しくユニットのデータを生成する
        if(!unitIdCheckList[unitId]){
          Unit unit = createUnit(unitId, y, x, hp, unitType);
          unitList[unitId] = unit;
        }else{
          updateUnit(unitId, y, x, hp);
        }
      }

      // 視野内の敵軍のユニット数
      scanf("%d", &enemyUnitCount);

      // 敵軍ユニットの詳細
      for(int i = 0; i < enemyUnitCount; i++){
        scanf("%d %d %d %d %d", &unitId, &y, &x, &hp, &unitType);
      }

      // 視野内の資源の数
      scanf("%d", &resourceCount);

      // 資源マスの詳細
      for(int i = 0; i < resourceCount; i++){
        scanf("%d %d", &y, &x);
      }

      // 終端文字列
      cin >> str;
    }

    /*
     * ユニットの作成を行う
     */
    Unit createUnit(int unitId, int y, int x, int hp, int unitType){
      Unit unit;
      unit.id           = unitId;
      unit.y            = y;
      unit.x            = x;
      unit.hp           = hp;
      unit.type         = unitType;
      unit.attackRange  = unitAttackRange[unitType];
      unit.eyeRange     = unitEyeRange[unitType];
      unit.canMove      = unitCanMove[unitType];

      return unit;
    }

    /*
     * ユニットの更新を行う(座標と残りHP)
     */
    void updateUnit(int unitId, int y, int x, int hp){
      Unit *unit = &unitList[unitId];
      unit->y   = y;
      unit->x   = x;
      unit->hp  = hp;
    }

    void run(){
      init();

      // 残り時間(ms)が取得出来なくなるまで回し続ける
      while(cin >> remainingTime){
        fprintf(stderr, "Remaing time is %dms\n", remainingTime);

        eachTurnProc();

        finalOperation();
      }
    }

    /*
     * 最終指示(このターンの最終的な行動を出力)
     */
    void finalOperation(){
      printf("0\n");
    }

    /*
     * ユニットに対して指示を出す
     */
    void operation(int id, int type){
      switch(type){
        case MOVE_UP:
          moveUp(id);
          break;
        case MOVE_DOWN:
          moveDown(id);
          break;
        case MOVE_LEFT:
          moveLeft(id);
          break;
        case MOVE_RIGHT:
          moveRight(id);
          break;
        case CREATE_WORKER:
          break;
        case CREATE_KNIGHT:
          break;
        case CREATE_FIGHER:
          break;
        case CREATE_ASSASIN:
          break;
        case CREATE_VILLAGE:
          break;
        case CREATE_BASE:
          break;
        default:
          noMove();
          break;
      }
    }

    /*
     * 何も行動しない
     */
    void noMove(){
    }

    /*
     * 上に動く
     */
    void moveUp(int id){
    }

    /*
     * 下に動く
     */
    void moveDown(int id){
    }

    /*
     * 左に動く
     */
    void moveLeft(int id){
    }

    /*
     * 右に動く
     */
    void moveRight(int id){
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
      return absDist[x1*WIDTH+x2] + absDist[y1*WIDTH+y2];
    }

    /*
     * フィールドの表示
     */
    void showField(){
      for(int y = 0; y < HEIGHT; y++){
        for(int x = 0; x < WIDTH; x++){
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
    fprintf(stderr, "TestCase2: %s\n", testCase2()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase3: %s\n", testCase3()? "SUCCESS!" : "FAILED!");
  }

  /*
   * マンハッタン距離が取得出来ているかどうかの確認
   */
  bool testCase1(){
    if(cv.calcDist(0,0,1,1) != 2) return false;
    if(cv.calcDist(0,0,0,0) != 0) return false;
    if(cv.calcDist(99,99,99,99) != 0) return false;
    if(cv.calcDist(0,99,99,0) != 198) return false;
    if(cv.calcDist(3,20,9,19) != 7) return false;

    return true;
  }

  /*
   * ステージの初期化が成功しているかどうかの確認
   */
  bool testCase2(){
    unitIdCheckList.clear();

    unitIdCheckList[1] = true;
    if(unitIdCheckList.size() != 1) return false;
    cv.stageInitialize();
    if(unitIdCheckList.size() != 0) return false;

    return true;
  }

  /*
   * サンプル入力がしっかりと取れているかどうか
   */
  bool testCase3(){
    if(stageNumber != 0) return false;
    if(turn != 27) return false;
    if(myResourceCount != 29) return false;
    if(myUnitCount != 13) return false;
    if(enemyUnitCount != 0) return false;
    if(resourceCount != 1) return false;

    return true;
  }
};

int main(){
  Codevs cv;
  CodevsTest cvt;

  cv.run();
  //cvt.testRun();

  return 0;
}
