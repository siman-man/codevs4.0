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
const int CREATE_BASE     = 10; // 拠点を生産

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

// 各ユニットへの命令
const char instruction[OPERATION_MAX] = {'X','U','D','L','R','0','1','2','3','5','6'};
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

// ユニットへの指示
struct Operation{
  short unitId;       // ユニットID
  char  operation;    // 命令のリスト
  int   evaluation;   // 命令の評価値

  bool operator >(const Operation &e) const{
    return evaluation < e.evaluation;
  }    
};

// ユニットが持つ属性
struct Unit{
  short id;           // ユニットのID
  char  mode;         // ユニットの状態
  char  y;            // y座標
  char  x;            // x座標
  int   hp;           // HP
  char  type;         // ユニットの種別
  char  eyeRange;     // 視野
  char  attackRange;  // 攻撃範囲
  bool  movable;      // 移動できるかどうか
  short timestamp;    // 更新ターン

  int calcEvaluation(){
    return 0;
  }
};

// フィールドの1マスに対応する
struct Node{
  bool opened;            // 一度調査したことがあるかどうか
  char myUnitCount[7];    // 自軍の各ユニット数
  char enemyUnitCount[7]; // 相手の各ユニット数
  short seenCount;        // 自軍のユニットがノードを監視している数
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
int myAllUnitCount;           // 自軍のユニット数
int enemyAllUnitCount;        // 敵軍のユニット数
int resourceCount;            // 資源の数
int myResourceCount;          // 自軍の資源の数
Unit unitList[MAX_UNIT_ID];   // ユニットのリスト
set<short> myActiveUnitList;  // 生存している自軍のユニットIDリスト

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

      // アクティブユニットリストの初期化
      myActiveUnitList.clear();

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
          addUnit(unitId, y, x, hp, unitType);
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
          addUnit(unitId, y, x, hp, unitType);
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
    void addUnit(int unitId, int y, int x, int hp, int unitType){
      Unit unit;
      unit.id           = unitId;
      unit.y            = y;
      unit.x            = x;
      unit.hp           = hp;
      unit.type         = unitType;
      unit.attackRange  = unitAttackRange[unitType];
      unit.eyeRange     = unitEyeRange[unitType];
      unit.movable      = unitCanMove[unitType];
      unit.mode         = NONE;
      unit.timestamp    = turn;

      unitList[unitId] = unit;
      myActiveUnitList.insert(unitId);
    }

    /*
     * ノードの作成を行う
     */
    Node createNode(){
      Node node;
      memset(node.myUnitCount, 0, sizeof(node.myUnitCount));
      memset(node.enemyUnitCount, 0, sizeof(node.enemyUnitCount));
      node.seenCount = 0;
      node.opened = false;

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
      openNode(y, x, unitEyeRange[unitType]);
    }

    /*
     * ユニットの削除を行う
     */
    void deleteUnit(int y, int x, int unitType){
      gameStage.field[y][x].myUnitCount[unitType] -= 1;
      myResourceCount += unitCost[unitType];
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
      unit->y         = y;
      unit->x         = x;
      unit->hp        = hp;
      unit->timestamp = turn;
    }

    /*
     * 自軍の生存確認
     * ユニットのtimestampが更新されていない場合は前のターンで的に倒されたので、
     * リストから排除する。
     */
    void unitSurvivalCheck(){
      set<short> tempList = myActiveUnitList;
      set<short>::iterator it = tempList.begin();

      while(it != tempList.end()){
        Unit *unit = &unitList[*it];

        if(unit->timestamp != turn){
          myActiveUnitList.erase(unit->id);
        }

        it++;
      }
    }

    /*
     * ゲームの実行
     */
    void run(){
      init();

      // 残り時間(ms)が取得出来なくなるまで回し続ける
      while(cin >> remainingTime){
        fprintf(stderr, "Remaing time is %dms\n", remainingTime);

        // 各ターンで行う処理(主に入力の処理)
        eachTurnProc();

        // 自軍の生存確認
        unitSurvivalCheck();

        // 行動フェーズ
        vector<Operation> operationList = actionPhase();

        // 最終的な出力
        finalOperation(operationList);
      }
    }

    /*
     * 最終指示(このターンの最終的な行動を出力)
     */
    void finalOperation(vector<Operation> &operationList){
      int size = operationList.size();

      printf("%d\n", size);
      for(int i = 0; i < size; i++){
        Operation ope = operationList[i];
        printf("%d %c\n", ope.unitId, ope.operation);
      }
    }

    /*
     * 視界をオープンする
     */
    void openNode(int ypos, int xpos, int eyeRange){
      for(int y = max(0, ypos-eyeRange); y <= min(HEIGHT-1, ypos+eyeRange); y++){
        int diff = 2*abs(ypos-y)/2;

        for(int x = max(0, xpos-eyeRange+diff); x <= min(WIDTH-1, xpos+eyeRange-diff); x++){
          if(isWall(y,x)) continue;

          gameStage.field[y][x].seenCount += 1;
          bool opened = (gameStage.field[y][x].seenCount > 0);

          //fprintf(stderr,"y = %d, x = %d, value = %d\n", y, x, gameStage.field[y][x].opened ^ opened);

          gameStage.openNodeCount += gameStage.field[y][x].opened ^ opened;
          gameStage.field[y][x].opened = opened;
        }
      }
    }

    /*
     * ユニットが行動を起こす
     */
    void unitAction(int unitId, int type){
      Unit *unit = &unitList[unitId];

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
          createUnit(unit->y, unit->x, WORKER);
          break;
        case CREATE_KNIGHT:
          createUnit(unit->y, unit->x, KNIGHT);
          break;
        case CREATE_FIGHER:
          createUnit(unit->y, unit->x, FIGHER);
          break;
        case CREATE_ASSASIN:
          createUnit(unit->y, unit->x, ASSASIN);
          break;
        case CREATE_VILLAGE:
          createUnit(unit->y, unit->x, VILLAGE);
          break;
        case CREATE_BASE:
          createUnit(unit->y, unit->x, BASE);
          break;
        default:
          noMove();
          break;
      }
    }

    /*
     * ユニットのアクションの取消を行う
     * unitId: ユニットID
     *   type: アクションの種類
     */
    void rollbackAction(int unitId, int type){
      Unit *unit = &unitList[unitId];

      switch(type){
        case MOVE_UP:
          moveDown(unit->id);
          break;
        case MOVE_DOWN:
          moveUp(unit->id);
          break;
        case MOVE_LEFT:
          moveRight(unit->id);
          break;
        case MOVE_RIGHT:
          moveLeft(unit->id);
          break;
        case CREATE_WORKER:
          createUnit(unit->y, unit->x, WORKER);
          break;
        case CREATE_KNIGHT:
          createUnit(unit->y, unit->x, KNIGHT);
          break;
        case CREATE_FIGHER:
          createUnit(unit->y, unit->x, FIGHER);
          break;
        case CREATE_ASSASIN:
          createUnit(unit->y, unit->x, ASSASIN);
          break;
        case CREATE_VILLAGE:
          createUnit(unit->y, unit->x, VILLAGE);
          break;
        case CREATE_BASE:
          createUnit(unit->y, unit->x, BASE);
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
     * 自軍に対して各種行動を選択する
     */
    vector<Operation> actionPhase(){
      set<short>::iterator it = myActiveUnitList.begin();
      vector<Operation> operationList;

      // 各ユニット毎に処理を行う
      while(it != myActiveUnitList.end()){
        Unit *unit = &unitList[*it];
        priority_queue<Operation, vector<Operation>, greater<Operation> > que;

        fprintf(stderr, "unitId = %d\n", unit->id);

        Operation ope;
        ope.unitId = unit->id;
        ope.operation = instruction[MOVE_RIGHT];

        operationList.push_back(ope);

        it++;
      }

      return operationList;
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
  void runTest(){
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
    fprintf(stderr, "TestCase13:\t%s\n", testCase13()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase14:\t%s\n", testCase14()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase15:\t%s\n", testCase15()? "SUCCESS!" : "FAILED!");
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
    if(myActiveUnitList.size() != 0) return false;
    if(gameStage.openNodeCount != 0) return false;

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
   * 「上に移動」が出来ているかどうか
   */
  bool testCase6(){
    Unit *unit = &unitList[0];
    int x = unit->x;
    int y = unit->y;

    cv.moveUp(unit->id);

    return (x == unit->x && y-1 == unit->y);
  }

  /*
   * 「下に移動」が出来ているかどうか
   */
  bool testCase7(){
    Unit *unit = &unitList[0];
    int x = unit->x;
    int y = unit->y;

    cv.moveDown(unit->id);

    return (x == unit->x && y+1 == unit->y);
  }

  /*
   * 「左に移動」が出来ているかどうか
   */
  bool testCase8(){
    Unit *unit = &unitList[0];
    int x = unit->x;
    int y = unit->y;

    cv.moveLeft(unit->id);

    return (x-1 == unit->x && y == unit->y);
  }

  /*
   * 「右に移動」が来ているかどうか
   */
  bool testCase9(){
    Unit *unit = &unitList[0];
    int x = unit->x;
    int y = unit->y;

    cv.moveRight(unit->id);

    return (x+1 == unit->x && y == unit->y);
  }

  /*
   * 「生産可否判定」が出来ているかどうか
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
   * Case11: ユニットが作成できるどうかの確認
   */
  bool testCase11(){
    cv.stageInitialize();

    myResourceCount = 40;
    cv.createUnit(5,5,WORKER);
    if(myResourceCount != 0) return false;
    if(gameStage.field[5][5].myUnitCount[WORKER] != 1) return false;
    if(gameStage.openNodeCount != 41) return false;

    myResourceCount = 20;
    cv.createUnit(1,1,KNIGHT);
    if(myResourceCount != 0) return false;
    if(gameStage.field[1][1].myUnitCount[KNIGHT] != 1) return false;
    if(gameStage.openNodeCount != 60) return false;

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
   * ノードの作成が出来ているかどうか
   */
  bool testCase12(){
    Node node = cv.createNode();

    if(node.opened) return false;
    if(node.myUnitCount[WORKER] != 0) return false;
    if(node.myUnitCount[BASE] != 0) return false;
    if(node.enemyUnitCount[WORKER] != 0) return false;
    if(node.enemyUnitCount[BASE] != 0) return false;
    if(node.seenCount != 0) return false;

    return true;
  }

  /*
   * ユニットの追加が出来ているかどうかの確認
   */
  bool testCase13(){
    int unitId = 100;
    cv.addUnit(unitId, 10, 10, 1980, WORKER);
    if(unitList[unitId].type != WORKER) return false;
    if(unitList[unitId].hp != 1980) return false;
    if(unitList[unitId].mode != NONE) return false;
    if(!unitList[unitId].movable) return false;

    unitId = 101;
    cv.addUnit(unitId, 50, 50, 20000, VILLAGE);
    if(unitList[unitId].type != VILLAGE) return false;
    if(unitList[unitId].hp != 20000) return false;
    if(unitList[unitId].movable) return false;

    unitId = 102;
    cv.addUnit(unitId, 30, 30, 20000, BASE);
    if(unitList[unitId].type != BASE) return false;
    if(unitList[unitId].hp != 20000) return false;
    if(unitList[unitId].movable) return false;

    if(myActiveUnitList.size() != 3) return false;


    return true;
  }

  /*
   * ユニットの生存確認が出来ているかどうかの確認
   */
  bool testCase14(){
    int unitId = 100;
    cv.stageInitialize();

    cv.addUnit(unitId, 10, 10, 1980, WORKER);

    unitId = 101;
    cv.addUnit(unitId, 20, 20, 1980, WORKER);
    unitList[unitId].timestamp = -1;

    cv.unitSurvivalCheck();

    if(myActiveUnitList.size() != 1) return false;

    return true;
  }

  /*
   * ユニットの削除が出来ているかどうかの確認
   */
  bool testCase15(){
    int unitId = 100;
    cv.stageInitialize();

    myResourceCount = 40;
    cv.createUnit(0,0,WORKER);

    cv.deleteUnit(0,0,WORKER);
    if(myResourceCount != 40) return false;
    if(gameStage.field[0][0].myUnitCount[WORKER] != 0) return false;

    return true;
  }
};

int main(){
  Codevs cv;
  CodevsTest cvt;

  cv.run();
  cvt.runTest();

  return 0;
}
