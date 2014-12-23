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
const int NO_MOVE         =  0; // 何も移動しない
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

// ユニットの行動タイプ
const int NONE    = 0;  // 何もしない(何も出来ない)
const int SEARCH  = 1;  // 探索(空いてないマスを探索)
const int DESTROY = 2;  // 破壊(敵を見つけて破壊)
const int PICKING = 3;  // 資源採取
const int ONRUSH  = 4;  // 突撃(敵の城が見つかり倒しに行く状態)

// 各種最大値
const int OPERATION_MAX = 11;   // 行動の種類
const int UNIT_MAX = 7;         // ユニットの種類
const int COST_MAX = 99999;     // コストの最大値(城を事実上作れなくする)

const int dy[5] = {0,-1, 1, 0, 0};
const int dx[5] = {0, 0, 0,-1, 1};

// 各ユニットの生産にかかるコスト(上の「行動一覧」と一致させておく)
const int unitCost[OPERATION_MAX] = {40, 20, 40, 60, COST_MAX, 100, 500};
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

int absDist[WIDTH*WIDTH];   // マンハッタン距離の出力

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
const bool OPERATION_LIST[7][12] = {
  /*        動無   動上   動下   動左   動右   産労   産騎   産闘   産殺   産城  産村   産拠 */
  /* 労 */ {true,  true,  true,  true,  true, false, false, false, false, false, true,  true},
  /* 騎 */ {true,  true,  true,  true,  true, false, false, false, false, false, false, false},
  /* 闘 */ {true,  true,  true,  true,  true, false, false, false, false, false, false, false},
  /* 殺 */ {true,  true,  true,  true,  true, false, false, false, false, false, false, false},
  /* 城 */ {true, false, false, false, false,  true, false, false, false, false, false, false},
  /* 村 */ {true, false, false, false, false,  true, false, false, false, false, false, false},
  /* 拠 */ {true, false, false, false, false, false,  true,  true,  true, false, false, false}
};

// ユニットが持つ属性
struct Unit{
  int   id;           // ユニットのID
  int   mode;         // ユニットの状態
  char  y;            // y座標
  char  x;            // x座標
  int   hp;           // HP
  int   type;         // ユニットの種別
  int   eyeRange;     // 視野
  int   attackRange;  // 攻撃範囲
  bool  movable;      // 移動できるかどうか

  int calcEvaluation(){
    return 0;
  }
};

// フィールドの1マスに対応する
struct Node{
  bool opened;            // 一度調査したことがあるかどうか
  char myUnitCount[7];    // 自軍の各ユニット数
  char enemyUnitCount[7]; // 相手の各ユニット数
};

// ゲーム・フィールド全体の構造
struct GameStage{
  short openNodeCount;
  Node field[HEIGHT][WIDTH];        // ゲームフィールド
};

int remainingTime;            // 残り時間
int stageNumber;              // 現在のステージ数
int currentStageNumber;       // 現在のステージ数
int turn;                     // 現在のターン
int myAllUnitCount;              // 自軍のユニット数
int enemyAllUnitCount;           // 敵軍のユニット数
int resourceCount;            // 資源の数
int myResourceCount;          // 自軍の資源の数
Unit unitList[MAX_UNIT_ID];   // ユニットのリスト

bool walls[HEIGHT+2][WIDTH+2];    // 壁かどうかを確認するだけのフィールド
Node tempField[HEIGHT][WIDTH];    // 一時的なゲームフィールド
map<int, bool> unitIdCheckList;   // IDが存在しているかどうかのチェック

GameStage gameStage;  // ゲームフィールド

class Codevs{
  public:
    /*
     * ゲームの初期化処理
     */
    void init(){
      currentStageNumber = -1;

      absDistInitialize();

      // 壁判定の初期化処理
      for(int y = 0; y <= HEIGHT+1; y++){
        for(int x = 0; x <= WIDTH+1; x++){
          walls[y][x] = (y == 0 || x == 0 || y == HEIGHT+1 || x == WIDTH + 1);
        }
      }

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
      // ユニットのチェックリストの初期化
      unitIdCheckList.clear();

      // 探索が完了したマスの初期化
      gameStage.openNodeCount = 0;

      // フィールドの初期化
      for(int y = 0; y < HEIGHT; y++){
        for(int x = 0; x < WIDTH; x++){
          gameStage.field[y][x] = createNode();
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
      scanf("%d", &myAllUnitCount);

      // 自軍ユニットの詳細
      for(int i = 0; i < myAllUnitCount; i++){
        scanf("%d %d %d %d %d", &unitId, &y, &x, &hp, &unitType);

        // チェックリストに載っていない場合は、新しくユニットのデータを生成する
        if(!unitIdCheckList[unitId]){
          unitList[unitId] = addUnit(unitId, y, x, hp, unitType);
        }else{
          updateUnit(unitId, y, x, hp);
        }
      }

      // 視野内の敵軍のユニット数
      scanf("%d", &enemyAllUnitCount);

      // 敵軍ユニットの詳細
      for(int i = 0; i < enemyAllUnitCount; i++){
        scanf("%d %d %d %d %d", &unitId, &y, &x, &hp, &unitType);

        // チェックリストに載っていない場合は、新しくユニットのデータを生成する
        if(!unitIdCheckList[unitId]){
          unitList[unitId] = addUnit(unitId, y, x, hp, unitType);
        }else{
          updateUnit(unitId, y, x, hp);
        }
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
     * ユニットの追加を行う
     */
    Unit addUnit(int unitId, int y, int x, int hp, int unitType){
      Unit unit;
      unit.id           = unitId;
      unit.y            = y;
      unit.x            = x;
      unit.hp           = hp;
      unit.type         = unitType;
      unit.attackRange  = unitAttackRange[unitType];
      unit.eyeRange     = unitEyeRange[unitType];
      unit.movable      = unitCanMove[unitType];

      return unit;
    }

    /*
     * ノードの作成を行う
     */
    Node createNode(){
      Node node;
      memset(node.myUnitCount, 0, sizeof(node.myUnitCount));
      memset(node.enemyUnitCount, 0, sizeof(node.enemyUnitCount));

      return node;
    }

    /*
     * ユニットの作成を行う
     *        y: y座標
     *        x: x座標
     * unitType: 生産するユニットの種類
     */
    void createUnit(int y, int x, int unitType){
      gameStage.field[y][x].myUnitCount[unitType] += 1;
      myResourceCount -= unitCost[unitType];
    }

    /*
     * ユニットの更新を行う(座標と残りHP)
     * unitId: ユニットのID
     *      y: y座標
     *      x: x座標
     *     hp: HP
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
    void operation(int unitId, int type){
      switch(type){
        case MOVE_UP:
          moveUp(unitId);
          break;
        case MOVE_DOWN:
          moveDown(unitId);
          break;
        case MOVE_LEFT:
          moveLeft(unitId);
          break;
        case MOVE_RIGHT:
          moveRight(unitId);
          break;
        case CREATE_WORKER:
          createWorker(unitId);
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
    void moveUp(int unitId){
      unitList[unitId].y -= 1;
    }

    /*
     * 下に動く
     */
    void moveDown(int unitId){
      unitList[unitId].y += 1;
    }

    /*
     * 左に動く
     */
    void moveLeft(int unitId){
      unitList[unitId].x -= 1;
    }

    /*
     * 右に動く
     */
    void moveRight(int unitId){
      unitList[unitId].x += 1;
    }

    /*
     * ワーカーの生産
     */
    void createWorker(int unitid){
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
     * 渡された座標が壁かどうかを判定する。
     * y: y座標
     * x: x座標
     */
    bool isWall(int y, int x){
      return walls[y+1][x+1];
    }

    /*
     * 移動が出来るかどうかのチェックを行う
     * y: y座標
     * x: x座標
     */
    bool canMove(int y, int x, int direct){
      int ny = y + dy[direct];
      int nx = x + dx[direct];

      return !isWall(ny,nx);
    }

    /*
     * ユニットの生産が可能かどうか
     * buildType: 生産したい物
     * unitTType: ユニットの種類
     */
    bool canBuild(int unitType, int buildType){
      return (OPERATION_LIST[unitType][buildType+5] && unitCost[buildType] <= myResourceCount);
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
    fprintf(stderr, "TestCase1:\t%s\n", testCase1()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase2:\t%s\n", testCase2()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase3:\t%s\n", testCase3()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase4:\t%s\n", testCase4()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase5:\t%s\n", testCase5()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase6:\t%s\n", testCase6()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase7:\t%s\n", testCase7()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase8:\t%s\n", testCase8()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase9:\t%s\n", testCase9()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase10:\t%s\n", testCase10()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase11:\t%s\n", testCase11()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase12:\t%s\n", testCase12()? "SUCCESS!" : "FAILED!");
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
    if(myAllUnitCount != 13) return false;
    if(enemyAllUnitCount != 0) return false;
    if(resourceCount != 1) return false;

    return true;
  }

  /*
   * 壁判定がちゃんと出来ているかどうか
   */
  bool testCase4(){
    if(!cv.isWall(-1,-1)) return false;
    if(!cv.isWall(-1, 0)) return false;
    if(!cv.isWall(HEIGHT,WIDTH)) return false;
    if(!cv.isWall(HEIGHT-1,WIDTH)) return false;
    if(cv.isWall(10,10)) return false;
    if(cv.isWall(0,0)) return false;
    if(cv.isWall(0,WIDTH-1)) return false;
    if(cv.isWall(HEIGHT-1,0)) return false;
    if(cv.isWall(HEIGHT-1,WIDTH-1)) return false;

    return true;
  }

  /*
   * 移動判定が出来ているかどうか
   */
  bool testCase5(){
    if(cv.canMove(0,0,MOVE_UP)) return false;
    if(cv.canMove(0,0,MOVE_LEFT)) return false;
    if(!cv.canMove(0,0,MOVE_DOWN)) return false;
    if(!cv.canMove(0,0,MOVE_RIGHT)) return false;
    if(!cv.canMove(0,0,NO_MOVE)) return false;

    return true;
  }

  /*
   * 「上に移動」がちゃんと出来ているかどうか
   */
  bool testCase6(){
    Unit *unit = &unitList[0];
    int x = unit->x;
    int y = unit->y;

    cv.moveUp(unit->id);

    return (x == unit->x && y-1 == unit->y);
  }

  /*
   * 「下に移動」がちゃんと出来ているかどうか
   */
  bool testCase7(){
    Unit *unit = &unitList[0];
    int x = unit->x;
    int y = unit->y;

    cv.moveDown(unit->id);

    return (x == unit->x && y+1 == unit->y);
  }

  /*
   * 「左に移動」がちゃんと出来ているかどうか
   */
  bool testCase8(){
    Unit *unit = &unitList[0];
    int x = unit->x;
    int y = unit->y;

    cv.moveLeft(unit->id);

    return (x-1 == unit->x && y == unit->y);
  }

  /*
   * 「右に移動」がちゃんと出来ているかどうか
   */
  bool testCase9(){
    Unit *unit = &unitList[0];
    int x = unit->x;
    int y = unit->y;

    cv.moveRight(unit->id);

    return (x+1 == unit->x && y == unit->y);
  }

  /*
   * 「生産可否判定」がちゃんと出来ているかどうか
   */
  bool testCase10(){
    Unit *castel  = &unitList[0];
    Unit *village = &unitList[1];
    Unit *base    = &unitList[2];
    Unit *worker  = &unitList[3];

    myResourceCount = 19;
    if(cv.canBuild(castel->type, KNIGHT)) return false;
    if(cv.canBuild(village->type, WORKER)) return false;
    if(cv.canBuild(base->type, KNIGHT)) return false;
    if(cv.canBuild(base->type, FIGHER)) return false;

    myResourceCount = 20;
    if(!cv.canBuild(base->type, KNIGHT)) return false;
    if(cv.canBuild(worker->type, KNIGHT)) return false;

    myResourceCount = 40;
    if(!cv.canBuild(village->type, WORKER)) return false;
    if(!cv.canBuild(castel->type, WORKER)) return false;
    if(!cv.canBuild(base->type, KNIGHT)) return false;
    if(!cv.canBuild(base->type, FIGHER)) return false;
    if(cv.canBuild(base->type, ASSASIN)) return false;

    myResourceCount = 60;
    if(!cv.canBuild(base->type, ASSASIN)) return false;
    if(cv.canBuild(castel->type, ASSASIN)) return false;
    if(cv.canBuild(worker->type, VILLAGE)) return false;

    myResourceCount = 100;
    if(!cv.canBuild(worker->type, VILLAGE)) return false;
    if(cv.canBuild(worker->type, BASE)) return false;

    myResourceCount = 500;
    if(!cv.canBuild(worker->type, BASE)) return false;
    if(cv.canBuild(village->type, BASE)) return false;

    return true;
  }

  /*
   * ユニットが作成できるどうかの確認
   */
  bool testCase11(){
    myResourceCount = 40;
    cv.createUnit(0,0,WORKER);
    if(myResourceCount != 0) return false;
    if(gameStage.field[0][0].myUnitCount[WORKER] != 1) return false;

    myResourceCount = 20;
    cv.createUnit(1,1,KNIGHT);
    if(myResourceCount != 0) return false;
    if(gameStage.field[1][1].myUnitCount[KNIGHT] != 1) return false;

    myResourceCount = 100;
    cv.createUnit(20,20,VILLAGE);
    if(myResourceCount != 0) return false;
    if(gameStage.field[20][20].myUnitCount[VILLAGE] != 1) return false;

    myResourceCount = 500;
    cv.createUnit(50,50,BASE);
    if(myResourceCount != 0) return false;
    if(gameStage.field[50][50].myUnitCount[BASE] != 1) return false;

    return true;
  }

  /*
   * ノードの作成がちゃんと出来ているかどうか
   */
  bool testCase12(){
    Node node = cv.createNode();

    if(node.opened) return false;
    if(node.myUnitCount[WORKER] != 0) return false;
    if(node.myUnitCount[BASE] != 0) return false;
    if(node.enemyUnitCount[WORKER] != 0) return false;
    if(node.enemyUnitCount[BASE] != 0) return false;

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
