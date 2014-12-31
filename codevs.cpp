#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
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

// 役割一覧
const int WORKER          =  0; // ワーカー
const int KNIGHT          =  1; // ナイト
const int FIGHTER          =  2; // ファイター
const int ASSASIN         =  3; // アサシン
const int CASTEL          =  4; // 城
const int VILLAGE         =  5; // 村
const int BASE            =  6; // 拠点
const int COMBATANT       =  7; // 戦闘員
const int LEADER          =  8; // 戦闘隊長
const int SPY             =  9; // スパイ
const int COLLIERY        = 10; // 炭鉱(資源マスにワーカーが5人いる状態)
const int VILLAGE_BREAKER = 11; // 相手の村をひたすら破壊する族

// 行動の基本優先順位
int movePriority[10] = { 5, 9, 8, 7, 0, 10, 15, 17, 20, 1};

// 行動一覧
const int NO_MOVE         =  0; // 何も移動しない
const int MOVE_UP         =  1; // 上に移動
const int MOVE_DOWN       =  2; // 下に移動
const int MOVE_LEFT       =  3; // 左に移動
const int MOVE_RIGHT      =  4; // 右に移動
const int CREATE_WORKER   =  5; // ワーカーを生産
const int CREATE_KNIGHT   =  6; // ナイトを生産
const int CREATE_FIGHTER   =  7; // ファイターを生産
const int CREATE_ASSASIN  =  8; // アサシンを生産
const int CREATE_CASTEL   =  9; // 城を生産
const int CREATE_VILLAGE  = 10; // 村を生産
const int CREATE_BASE     = 11; // 拠点を生産

// 試合状況一覧
const int OPENING = 0;  // 序盤戦
const int WARNING = 1;  // 敵ユニットを検知
const int DANGER  = 2;  // 自軍の白の視野に敵を確認


// ユニットの行動タイプ
const int NONE    = 0;  // 何もしない(何も出来ない)
const int SEARCH  = 1;  // 探索(空いてないマスを探索)
const int DESTROY = 2;  // 破壊(敵を見つけて破壊)
const int PICKING = 3;  // 資源採取
const int ONRUSH  = 4;  // 突撃(敵の城が見つかり倒しに行く状態)
const int STAY    = 5;  // 待機命令

// 各種最大値
const int OPERATION_MAX = 12;   // 行動の種類
const int UNIT_MAX = 7;         // ユニットの種類
const int COST_MAX = 99999;     // コストの最大値
const int MIN_VALUE = -999999;  // 最小値
const int MAX_VALUE = 999999;   // 最小値

// 座標計算で使用する配列
const int dy[5] = {0,-1, 1, 0, 0};
const int dx[5] = {0, 0, 0,-1, 1};

// その他
const int UNKNOWN = -1;       // 未知
const int UNDEFINED = -1;     // 未定
const int VIRTUAL_ID = 30000; // 仮想ID 
const int REAL = true;        // 確定コマンド
int ATTACK_NUM = 40;          // 突撃を仕掛ける人数
int attackCount = 0;          // 突撃した回数

// 各ユニットへの命令
const char instruction[OPERATION_MAX] = {'X','U','D','L','R','0','1','2','3','4','5','6'};
// 各ユニットの生産にかかるコスト(上の「ユニット一覧」と一致させておく)
const int unitCost[UNIT_MAX] = {40, 20, 40, 60, COST_MAX, 100, 500};
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

int manhattanDist[WIDTH*WIDTH];   // マンハッタン距離の出力

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
const bool OPERATION_LIST[UNIT_MAX][OPERATION_MAX] = {
  /*        動無   動上   動下   動左   動右   産労   産騎   産闘   産殺   産城   産村   産拠 */
  /* 労 */ {true,  true,  true,  true,  true, false, false, false, false, false,  true,  true},
  /* 騎 */ {true,  true,  true,  true,  true, false, false, false, false, false, false, false},
  /* 闘 */ {true,  true,  true,  true,  true, false, false, false, false, false, false, false},
  /* 殺 */ {true,  true,  true,  true,  true, false, false, false, false, false, false, false},
  /* 城 */ {true, false, false, false, false,  true, false, false, false, false, false, false},
  /* 村 */ {true, false, false, false, false,  true, false, false, false, false, false, false},
  /* 拠 */ {true, false, false, false, false, false,  true,  true,  true, false, false, false}
};

// ユニットへの指示
struct Operation{
  int unitId;       // ユニットID
  int operation;    // 命令のリスト
  int evaluation;   // 命令の評価値

  bool operator >(const Operation &e) const{
    return evaluation < e.evaluation;
  }    
};

// 最良探索で使用する
struct Cell{
  int y;
  int x;
  int cost;

  Cell(int ypos, int xpos, int c = -1){
    y = ypos;
    x = xpos;
    cost = c;
  }

  bool operator >(const Cell &e) const{
    return cost > e.cost;
  }    
};

// 行動の優先順位
struct MovePriority{
  int unitId;   // ユニットID
  int priority; // 優先度

  MovePriority(int id = 0, int value = 0){
    unitId = id;
    priority = value;
  }

  bool operator >(const MovePriority &e) const{
    return priority < e.priority;
  }    
};

// フィールドの1マスに対応する
struct Node{
  bool resource;            // 資源マスかどうか
  bool opened;              // 調査予定マス
  bool searched;            // 既に調査済みかどうか
  bool rockon;              // ノードを狙っている自軍がいるかどうか
  bool nodamage;            // ダメージを受けていない
  bool enemyCastel;         // 敵の城がある可能性
  int stamp;                // 足跡
  int cost;                 // ノードのコスト
  int markCount;            // マークカウント(自軍が行動する予定のマス)
  int seenCount;            // ノードを監視しているユニットの数 
  int troopsId;             // 滞在中の軍隊ID
  int myUnitCount[7];       // 自軍の各ユニット数
  int enemyUnitCount[7];    // 相手の各ユニット数
  int enemyAttackCount[7];  // 敵の攻撃の数
  int timestamp;            // タイムスタンプ
  set<int> seenMembers;     // ノードを監視している自軍のメンバー
  set<int> myUnits;         // 自軍のIDリスト
};

// ユニットが持つ属性
struct Unit{
  int id;                 // ユニットのID
  int mode;               // ユニットの状態
  int y;                  // y座標
  int x;                  // x座標
  int role;               // 役割
  int destY;              // 目的地のy座標
  int destX;              // 目的地のx座標
  int leaderId;           // 隊長のID
  int troopsCount;        // 部隊の人数
  int troopsLimit;        // 突撃する人数
  Node *townId;           // 拠点にしてて村のID
  int resourceY;          // 目的地(資源)のy座標
  int resourceX;          // 目的地(資源)のx座標
  int createWorkerCount;  // 生産したワーカーの数
  int createKnightCount;  // 生産したナイトの数
  int castelAttackCount;  // 敵の城から攻撃を受けた回数
  int hp;                 // HP
  int beforeHp;           // 前のターンのHP
  int type;               // ユニットの種別
  int eyeRange;           // 視野
  int attackRange;        // 攻撃範囲
  bool  movable;          // 移動できるかどうか
  int timestamp;          // 更新ターン
};

// ゲーム・フィールド全体の構造
struct GameStage{
  int searchedNodeCount;            // 調査済みのマスの数
  int openedNodeCount;              // 調査予定マスの数
  int visibleNodeCount;             // 現在確保できている視界の数   
  int gameSituation;                // 試合状況
  int incomeResource;               // このターンに得られた収入
  int targetY;                      // 目的地
  int targetX;                      // 目的地
  int baseCount;                    // 拠点の数
  bool castelAttack;                // 城からの攻撃を受けている
  Node field[HEIGHT][WIDTH];        // ゲームフィールド
  queue<int> enemyCastelPointList;    // 敵の城の座標候補
};

// 座標を表す
struct Coord{
  int y;
  int x;
  int dist;

  Coord(int ypos = -1, int xpos = -1){
    y = ypos;
    x = xpos;
  }

  bool operator >(const Coord &e) const{
    return dist > e.dist;
  }    
};

typedef pair<Coord, int> cell;

int remainingTime;            // 残り時間
int stageNumber;              // 現在のステージ数
int currentStageNumber;       // 現在のステージ数
int turn;                     // 現在のターン
int totalTurn;                // ターンの合計値
int myAllUnitCount;           // 自軍のユニット数
int enemyAllUnitCount;        // 敵軍のユニット数
int resourceCount;            // 資源の数
int myResourceCount;          // 自軍の資源の数
int myCastelCoordY;           // 自軍の城のy座標
int myCastelCoordX;           // 自軍の城のx座標
int enemyCastelCoordY;        // 敵軍の城のy座標
int enemyCastelCoordX;        // 敵軍の城のx座標
Unit unitList[MAX_UNIT_ID];   // ユニットのリスト
set<int> myActiveUnitList;    // 生存している自軍のユニットのIDリスト
set<int> enemyActiveUnitList; // 生存している敵軍のユニットのIDリスト
set<int> resourceNodeList;    // 資源マスのリスト

bool walls[HEIGHT+2][WIDTH+2];    // 壁かどうかを確認するだけのフィールド
Node tempField[HEIGHT][WIDTH];    // 一時的なゲームフィールド
map<int, bool> unitIdCheckList;   // IDが存在しているかどうかのチェック

GameStage gameStage;      // ゲームフィールド
GameStage tempGameStage;  // 一時的なゲーム・フィールド

/*
 * メインのコード部分
 */
class Codevs{
  public:
    /*
     * ゲームの初期化処理
     */
    void init(){
      currentStageNumber = -1;
      totalTurn = 0;

      manhattanDistInitialize();

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
    void manhattanDistInitialize(){
      for(int y = 0; y < HEIGHT; y++){
        for(int x = 0; x < WIDTH; x++){
          int id = (y*WIDTH+x);
          int dist = abs(y-x);
          manhattanDist[id] = dist;
        } 
      }
    }

    /*
     * ステージ開始直前に行う初期化処理
     */
    void stageInitialize(){
      totalTurn += turn;
      fprintf(stderr,"total turn: %d\n", totalTurn);
      fprintf(stderr,"stageInitialize =>\n");
      // ユニットのチェックリストの初期化
      unitIdCheckList.clear();

      // アクティブユニットリストの初期化
      myActiveUnitList.clear();

      // 敵ユニットリストの初期化
      enemyActiveUnitList.clear();

      // 突撃回数の初期化
      attackCount = 0;

      // 資源マスの初期化
      resourceNodeList.clear();

      // 敵の城の座標候補リストの初期化
      queue<int> que;
      gameStage.enemyCastelPointList = que;

      // 拠点の数
      gameStage.baseCount = 0;

      // 城からの攻撃をfalse
      gameStage.castelAttack = false;

      // 最初に探索する部分を決める
      gameStage.targetY = 10;
      gameStage.targetX = 10;

      // ゲームの初期状態
      gameStage.gameSituation = OPENING;

      // 探索が完了したマスの初期化
      gameStage.searchedNodeCount = 0;

      // 確保している視界の数の初期化
      gameStage.visibleNodeCount = 0;

      // 調査予定のノード数の初期化
      gameStage.openedNodeCount = 0;

      // フィールドの初期化
      for(int y = 0; y < HEIGHT; y++){
        for(int x = 0; x < WIDTH; x++){
          gameStage.field[y][x] = createNode();
        }
      }

      // コストを付ける
      checkCost();

      // 自軍の城の座標をリセット
      myCastelCoordY = UNKNOWN;
      myCastelCoordX = UNKNOWN;

      // 敵軍の城の座標をリセット
      enemyCastelCoordY = UNKNOWN;
      enemyCastelCoordX = UNKNOWN;
    }

    /*
     * 各ターンの入力処理と初期化処理
     */
    void eachTurnProc(){
      int unitId;   // ユニットID
      int y;        // y座標
      int x;        // x座標
      int hp;       // HP
      int unitType; // ユニットの種類
      string str;   // 終端文字列「END」を格納するだけの変数

      // 調査予定のノード数をリセット
      gameStage.openedNodeCount = 0;

      // 確保できている視野のリセット
      gameStage.visibleNodeCount = 0;

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

      if(currentStageNumber == 23 && turn % 10 == 0){
        fprintf(stderr,"totalTurn = %d\n", totalTurn + turn);
      }

      // 資源数
      scanf("%d", &myResourceCount);

      // 自軍のユニット数
      scanf("%d", &myAllUnitCount);

      // 自軍ユニットの詳細
      for(int i = 0; i < myAllUnitCount; i++){
        scanf("%d %d %d %d %d", &unitId, &y, &x, &hp, &unitType);


        // チェックリストに載っていない場合は、新しくユニットのデータを生成する
        if(!unitIdCheckList[unitId]){
          addMyUnit(unitId, y, x, hp, unitType);
        }else{
          updateMyUnitStatus(unitId, y, x, hp);
        }
      }

      // 視野内の敵軍のユニット数
      scanf("%d", &enemyAllUnitCount);

      // 敵軍ユニットの詳細
      for(int i = 0; i < enemyAllUnitCount; i++){
        scanf("%d %d %d %d %d", &unitId, &y, &x, &hp, &unitType);
        
        // チェックリストに載っていない場合は、新しくユニットのデータを生成する
        if(!unitIdCheckList[unitId]){
          addEnemyUnit(unitId, y, x, hp, unitType);
        }else{
          updateEnemyUnitStatus(unitId, y, x, hp);
        }
      }

      // 視野内の資源の数
      scanf("%d", &resourceCount);

      // 資源マスの詳細
      for(int i = 0; i < resourceCount; i++){
        scanf("%d %d", &y, &x);
        addResourceNode(y,x);
      }

      // 終端文字列
      cin >> str;
    }

    /*
     * 自軍ユニットの追加を行う
     *   unitId: ユニットID
     *        y: y座標
     *        x: x座標
     *       hp: HP
     * unitType: ユニットの種類
     */
    void addMyUnit(int unitId, int y, int x, int hp, int unitType){
      Unit unit;
      unit.id           = unitId;
      unit.y            = y;
      unit.x            = x;
      unit.hp           = hp;
      unit.beforeHp     = hp;
      unit.type         = unitType;
      unit.destY        = UNDEFINED;
      unit.destX        = UNDEFINED;
      unit.resourceY    = UNDEFINED;
      unit.resourceX    = UNDEFINED;
      unit.createWorkerCount = 0;
      unit.createKnightCount = 0;
      unit.castelAttackCount = 0;
      unit.leaderId     = UNDEFINED;
      unit.troopsCount  = 0;
      unit.attackRange  = unitAttackRange[unitType];
      unit.eyeRange     = unitEyeRange[unitType];
      unit.movable      = unitCanMove[unitType];
      unit.timestamp    = turn;

      // 自軍の城の座標を更新
      if(unitType == CASTEL){
        myCastelCoordY = y;
        myCastelCoordX = x;
      }

      gameStage.field[y][x].myUnitCount[unitType] += 1;
      gameStage.field[y][x].myUnits.insert(unitId);

      unitList[unitId] = unit;
      unitList[unitId].mode = directFirstMode(&unitList[unitId]);
      unitList[unitId].role = directUnitRole(&unitList[unitId]);
      myActiveUnitList.insert(unitId);
      unitIdCheckList[unitId] = true;
      checkNode(unitId, y, x, unit.eyeRange);

      if(unitList[unitId].mode == SEARCH){
        //checkStamp(y, x, unit.eyeRange * 2);
      }

      if(unitList[unitId].role == COMBATANT){
        searchLeader(&unitList[unitId]);
      }
    }

    /*
     * リーダを探し出す
     */
    void searchLeader(Unit *unit){
      //fprintf(stderr,"searchLeader =>\n");
      set<int>::iterator it = myActiveUnitList.begin();
      int minDist = MAX_VALUE;
      int dist;
      int leaderId = UNDEFINED;

      while(it != myActiveUnitList.end()){
        Unit *other = &unitList[*it];
        dist = calcManhattanDist(unit->y, unit->x, other->y, other->x);

        if(other->role == LEADER && minDist > dist){
          minDist = dist;
          leaderId = other->id;
        }

        it++;
      }

      unit->leaderId = leaderId;
      unitList[leaderId].troopsCount += 1;
    }

    /*
     * 自軍のユニットを削除する
     * unit: ユニット
     */
    void removeMyUnit(Unit *unit){
      myActiveUnitList.erase(unit->id);
      uncheckNode(unit->y, unit->x, unit->type, unit->eyeRange);
    } 

    /*
     * 敵軍のユニットを削除する
     */
    void removeEnemyUnit(Unit *unit){
      enemyActiveUnitList.erase(unit->id);
      unitIdCheckList[unit->id] = false;
    }

    /*
     * 敵軍ユニットの追加を行う
     *   unitId: ユニットID
     *        y: y座標
     *        x: x座標
     *       hp: HP
     * unitType: ユニットの種類
     */
    void addEnemyUnit(int unitId, int y, int x, int hp, int unitType){
      Unit unit;
      unit.id           = unitId;
      unit.y            = y;
      unit.x            = x;
      unit.hp           = hp;
      unit.type         = unitType;
      unit.attackRange  = unitAttackRange[unitType];
      unit.eyeRange     = unitEyeRange[unitType];
      unit.movable      = unitCanMove[unitType];
      unit.timestamp    = turn;

      Node *node = getNode(y, x);

      // 敵軍の城の座標を更新
      if(unitType == CASTEL){
        enemyCastelCoordY = y;
        enemyCastelCoordX = x;
      }

      node->enemyUnitCount[unitType] += 1;
      unitList[unitId] = unit;
      enemyActiveUnitList.insert(unitId);
      unitIdCheckList[unitId] = true;
    }

    /*
     * 資源マスの追加を行う
     */    
    void addResourceNode(int y, int x){
      gameStage.field[y][x].resource = true;
      resourceNodeList.insert(y*WIDTH+x);
    }

    /*
     * 資源マスが占領されているかどうかを調べる
     */
    bool isOccupied(int y, int x){
      Node *node = getNode(y, x);
      return (node->enemyUnitCount[WORKER] >= 4);
    }

    /*
     * 突撃する人数を変える
     */ 
    void updateTroopsLimit(Unit *unit){
      if(gameStage.gameSituation == ONRUSH){
        Node *node = getNode(enemyCastelCoordY, enemyCastelCoordX);

        if(node->enemyUnitCount[BASE] == 0 && attackCount <= 1){
          unit->troopsLimit = 20;
        }
      }
    }

    /*
     * 最初のモードを決める
     */
    int directFirstMode(Unit *unit){
      Node *node = &gameStage.field[unit->y][unit->x];
      updateTroopsLimit(unit);

      switch(unit->type){
        case WORKER:
          if(node->resource && node->myUnitCount[WORKER] <= 5){
            unit->resourceY = unit->y;
            unit->resourceX = unit->x;

            return PICKING;
          }else{
            return SEARCH;
          }
          break;
        case VILLAGE:
          return NONE;
          break;
        case KNIGHT:
          if(unit->troopsCount < unit->troopsLimit){
            return STAY;
          }else{
            return DESTROY;
          }
          break;
        case FIGHTER:
          if(unit->troopsCount < unit->troopsLimit){
            return STAY;
          }else{
            return DESTROY;
          }
          break;
        case ASSASIN:
          if(unit->troopsCount < unit->troopsLimit){
            return STAY;
          }else{
            return DESTROY;
          }
          break;
        default:
          break;
      }

      return NONE;
    }

    /*
     * 次の目的地を決める(指定したポイントから一番近い未探索地域)
     */
    Coord directNextPoint(Unit *unit){
      Coord bestCoord;

      if(gameStage.gameSituation == ONRUSH){
        assert(gameStage.gameSituation == ONRUSH && enemyCastelCoordY != UNDEFINED);
        assert(gameStage.gameSituation == ONRUSH && enemyCastelCoordX != UNDEFINED);
      }

      queue<Coord> que;
      que.push(Coord(unit->y, unit->x));
      map<int, bool> checkList;

      while(!que.empty()){
        Coord coord = que.front(); que.pop();

        if(checkList[coord.y*WIDTH+coord.x]) continue;
        checkList[coord.y*WIDTH+coord.x] = true;

        Node *node = &gameStage.field[coord.y][coord.x];

        if(!node->searched && node->markCount == 0){
          return coord;
        }

        for(int i = 1; i < 5; i++){
          int ny = coord.y + dy[i];
          int nx = coord.x + dx[i];

          if(!isWall(ny,nx)) que.push(Coord(ny,nx));
        }
      }

      return Coord(enemyCastelCoordY, enemyCastelCoordX);
    }

    /*
     * 収入を更新
     */
    void updateIncomeResource(){
      gameStage.incomeResource = 10;
      set<int>::iterator it = resourceNodeList.begin();

      while(it != resourceNodeList.end()){
        int y = (*it)/WIDTH;
        int x = (*it)%WIDTH;

        gameStage.incomeResource += min(5, gameStage.field[y][x].myUnitCount[WORKER]);

        it++;
      }
    }

    /*
     * 次の未開地を決める
     */
    void updateUnknownPoint(){
      Coord bestCoord;

      if(!gameStage.castelAttack && !gameStage.field[gameStage.targetY][gameStage.targetX].searched){
        //fprintf(stderr,"turn = %d, Not Yet Target Point Searched: y = %d, x = %d\n", turn, gameStage.targetY, gameStage.targetX);
        return;
      }else if(!gameStage.castelAttack && gameStage.targetX < 90 && gameStage.targetY < 90 && gameStage.targetY + 10 < 99 && gameStage.targetX + 10 < 99){
        gameStage.targetY += 10;
        gameStage.targetX += 10;
        //fprintf(stderr,"Next Target Point Searched +10: y = %d, x = %d\n", gameStage.targetY, gameStage.targetX);
        return;
      }else if(gameStage.castelAttack && gameStage.enemyCastelPointList.size() > 0){
        Node *node = &gameStage.field[gameStage.targetY][gameStage.targetX];

        if(node->searched || !node->enemyCastel){
          gameStage.enemyCastelPointList.pop();
          //fprintf(stderr,"turn = %d, Next Target Point Searched: y = %d, x = %d\n", turn, gameStage.targetY, gameStage.targetX);
        }else{
          return;
        }

        int id = gameStage.enemyCastelPointList.front();
        int y = id / WIDTH;
        int x = id % WIDTH;

        gameStage.targetY = y;
        gameStage.targetX = x;

        //fprintf(stderr,"Next Target Point: y = %d, x = %d\n", y, x);
        return;
      }else{
        if(gameStage.gameSituation == ONRUSH){
          assert(gameStage.gameSituation == ONRUSH && enemyCastelCoordY != UNDEFINED);
          assert(gameStage.gameSituation == ONRUSH && enemyCastelCoordX != UNDEFINED);
        }

        queue<Coord> que;
        que.push(Coord(70, 90));
        map<int, bool> checkList;

        while(!que.empty()){
          Coord coord = que.front(); que.pop();

          if(checkList[coord.y*WIDTH+coord.x]) continue;
          checkList[coord.y*WIDTH+coord.x] = true;

          Node *node = &gameStage.field[coord.y][coord.x];

          if(!node->searched && node->enemyCastel && calcManhattanDist(coord.y, coord.x, 99, 99) <= 40){
            //fprintf(stderr,"Next point: y = %d, x = %d\n", coord.y, coord.x);
            gameStage.targetY = coord.y;
            gameStage.targetX = coord.x;
            return;
          }

          for(int i = 1; i < 5; i++){
            int ny = coord.y + dy[i];
            int nx = coord.x + dx[i];

            if(!isWall(ny,nx)) que.push(Coord(ny,nx));
          }
        }

        gameStage.targetY = enemyCastelCoordY;
        gameStage.targetX = enemyCastelCoordX;
      }
    }

    /*
     * 目的地を決めて探索する
     */
    Coord directTargetPoint(int y, int x, int targetY, int targetX){
      map<int, bool> checkList;
      priority_queue< Cell, vector<Cell>, greater<Cell> > que;

      que.push(Cell(y, x, 0));

      while(!que.empty()){
        Cell cell = que.top(); que.pop();

        if(checkList[cell.y*WIDTH+cell.x]) continue;
        checkList[cell.y*WIDTH+cell.x] = true;

        Node *node = &gameStage.field[cell.y][cell.x];

        if(!node->searched && node->markCount == 0){
          return Coord(cell.y, cell.x);
        }

        for(int i = 1; i < 5; i++){
          int ny = cell.y + dy[i];
          int nx = cell.x + dx[i];

          if(!isWall(ny,nx)){
            int nc = calcManhattanDist(ny, nx, targetY, targetX) + node->stamp + node->cost;
            que.push(Cell(ny, nx, cell.cost + nc));
          }
        }
      }
    };

    /*
     * 未知領域の計算
     * 指定した座標の未知数を計算
     */
    int calcUnknownPoint(int ypos, int xpos){
      int point = calcManhattanDist(ypos, xpos, 0, 0);

      map<int, bool> checkList;
      queue<cell> que;
      que.push(cell(Coord(ypos,xpos),0));

      while(!que.empty()){
        cell c = que.front(); que.pop(); 
        Coord coord = c.first;
        int dist = c.second;

        if(isWall(coord.y, coord.x)){
          point -= 100;
          continue;
        }
        if(checkList[coord.y*WIDTH+coord.x] || dist >= 5) continue;
        checkList[coord.y*WIDTH+coord.x] = true;

        Node *node = &gameStage.field[coord.y][coord.x];

        //point += (gameStage.field[coord.y][coord.x].searched)? -10 : 1000; 
        if(!node->searched){
          point += gameStage.field[coord.y][coord.x].cost;
        }

        //point -= 4 * gameStage.field[coord.y][coord.x].markCount;

        for(int i = 1; i < 5; i++){
          int ny = coord.y + dy[i];
          int nx = coord.x + dx[i];

          que.push(cell(Coord(ny, nx), dist+1));
        }
      }

      return point;
    }

    /*
     * ノードの作成を行う
     */
    Node createNode(){
      Node node;
      memset(node.myUnitCount, 0, sizeof(node.myUnitCount));
      memset(node.enemyUnitCount, 0, sizeof(node.enemyUnitCount));
      memset(node.enemyAttackCount, 0, sizeof(node.enemyAttackCount));
      node.seenCount = 0;
      node.cost = 0;
      node.stamp = 0;
      node.markCount = 0;
      node.timestamp = 0;
      node.troopsId = UNDEFINED;
      node.resource = false;
      node.opened = false;
      node.rockon = false;
      node.searched = false;
      node.nodamage = false;
      node.enemyCastel = true;

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
     *        y: y座標
     *        x: x座標
     * unitType: ユニットの種類
     */
    void deleteUnit(int y, int x, int unitType){
      gameStage.field[y][x].myUnitCount[unitType] -= 1;
      myResourceCount += unitCost[unitType];
      closeNode(y, x, unitEyeRange[unitType]);
    }

    /*
     * ノードのタイムスタンプを更新する
     */
    void updateNodeTimestamp(int y, int x, int range = 4){
      map<int, bool> checkList;
      queue<cell> que;
      que.push(cell(Coord(y, x), 0));

      while(!que.empty()){
        cell c = que.front(); que.pop();

        Coord coord = c.first;
        int dist = c.second;

        if(checkList[coord.y*WIDTH+coord.x] || dist > range) continue;
        checkList[coord.y*WIDTH+coord.x] = true;

        Node *node = &gameStage.field[coord.y][coord.x];
        node->timestamp = turn;

        for(int i = 1; i < 5; i++){
          int ny = coord.y + dy[i];
          int nx = coord.x + dx[i];

          if(!isWall(ny,nx)) que.push(cell(Coord(ny,nx), dist+1));
        }
      }
    }

    /*
     * 自軍ユニットの状態の更新を行う(座標と残りHP)
     * unitId: ユニットのID
     *      y: y座標
     *      x: x座標
     *     hp: HP
     */
    void updateMyUnitStatus(int unitId, int y, int x, int hp){
      Unit *unit = &unitList[unitId];
      unit->y         = y;
      unit->x         = x;
      unit->beforeHp  = unit->hp;
      unit->hp        = hp;
      unit->timestamp = turn;

      Node *node = getNode(y, x);

      if(!node->nodamage && unit->beforeHp == unit->hp){
        node->nodamage = true; 
        checkNoEnemyCastel(y,x);
      }

      node->myUnitCount[unit->type] += 1;
      node->myUnits.insert(unitId);
    }

    /*
     * 敵軍ユニットの状態の更新を行う(座標と残りHP)
     * unitId: ユニットのID
     *      y: y座標
     *      x: x座標
     *     hp: HP
     */
    void updateEnemyUnitStatus(int unitId, int y, int x, int hp){
      Unit *unit = &unitList[unitId];
      unit->y         = y;
      unit->x         = x;
      unit->hp        = hp;
      unit->timestamp = turn;

      Node *node = getNode(y, x);

      node->enemyUnitCount[unit->type] += 1;
    }

    /*
     * ユニットのモードの状態の更新を行う
     */
    void updateUnitMode(){
      set<int>::iterator it = myActiveUnitList.begin();
      if(gameStage.gameSituation != ONRUSH){
        updateUnknownPoint();
      }

      while(it != myActiveUnitList.end()){
        Unit *unit = &unitList[*it];
        unit->mode = directUnitMode(unit);

        // SEARCHモードのユニットの目的地の設定されていない場合、更新する。
        if((unit->mode == SEARCH) && (gameStage.field[unit->destY][unit->destX].searched || (unit->destY == UNDEFINED && unit->destX == UNDEFINED))){
          if(enemyCastelCoordY != UNDEFINED && enemyCastelCoordX != UNDEFINED){
            unit->destY = enemyCastelCoordY;
            unit->destX = enemyCastelCoordX;
          }else if(!gameStage.gameSituation){
            Coord coord;
            coord = directTargetPoint(unit->y, unit->x, gameStage.targetY, gameStage.targetX);
            unit->destY = coord.y;
            unit->destX = coord.x;
          }else{
            unit->destY = gameStage.targetY;
            unit->destX = gameStage.targetX;
          }

          checkMark(unit->destY, unit->destX);
        }

        it++;
      }
    }

    /*
     * ユニットのモードを更新する
     */
    int directUnitMode(Unit *unit){
      Node *node = &gameStage.field[unit->y][unit->x];
      updateTroopsLimit(unit);

      switch(unit->type){
        case WORKER:
          if(unit->mode == PICKING || pickModeCheck(unit)){
            return PICKING;
          }else{
            return SEARCH;
          }
          break;
        case KNIGHT:
          switch(unit->role){
            case LEADER:
              if(unit->troopsCount >= unit->troopsLimit){
                if(unit->townId != NULL){
                  unit->townId = NULL;
                  node->troopsId = UNDEFINED;
                }

                return DESTROY;
              }else if(unit->mode == STAY && unit->troopsCount < unit->troopsLimit){
                return STAY;
              }else{
                return DESTROY;
              }
              break;
            default:
              Unit *leader = &unitList[unit->leaderId];
              if(leader->mode == DESTROY){
                return DESTROY;
              }else{
                return STAY;
              }
              break;
          }
          break;
        case FIGHTER:
          switch(unit->role){
            case LEADER:
              if(unit->troopsCount >= unit->troopsLimit){
                if(unit->townId != NULL){
                  unit->townId = NULL;
                  node->troopsId = UNDEFINED;
                }

                return DESTROY;
              }else if(unit->mode == STAY && unit->troopsCount < unit->troopsLimit){
                return STAY;
              }else{
                return DESTROY;
              }
              break;
            default:
              Unit *leader = &unitList[unit->leaderId];
              if(leader->mode == DESTROY){
                return DESTROY;
              }else{
                return STAY;
              }
              break;
          }
          break;
        case ASSASIN:
          switch(unit->role){
            case LEADER:
              if(unit->troopsCount >= unit->troopsLimit){
                if(unit->townId != NULL){
                  unit->townId = NULL;
                  node->troopsId = UNDEFINED;
                }

                return DESTROY;
              }else if(unit->mode == STAY && unit->troopsCount < unit->troopsLimit){
                return STAY;
              }else{
                return DESTROY;
              }
              break;
            default:
              Unit *leader = &unitList[unit->leaderId];
              if(leader->mode == DESTROY){
                return DESTROY;
              }else{
                return STAY;
              }
              break;
          }
          break;
        default:
          return NONE;
      }

      return NONE;
    }

    /*
     * 試合状況の更新を行う
     */
    void updateGameSituation(){
      if(enemyCastelCoordY != UNDEFINED && enemyCastelCoordX != UNDEFINED){
        gameStage.gameSituation = ONRUSH;
      }else if(gameStage.gameSituation != WARNING && enemyActiveUnitList.size() == 0){
        gameStage.gameSituation = OPENING;
      }else{
        gameStage.gameSituation = WARNING;
      }
    }

    /*
     * 周りにリーダが居ないか調べる
     */
    bool existLeader(int y, int x){
      for(int i = 0; i < 5; i++){
        int ny = y + dy[i];
        int nx = x + dx[i];

        if(!isWall(ny,nx) && gameStage.field[ny][nx].troopsId != UNDEFINED){
          return true;
        }
      }

      return false;
    }

    /*
     * 役割を決める
     */
    int directUnitRole(Unit *unit){
      Node *node = &gameStage.field[unit->y][unit->x];

      if(unit->type == ASSASIN || unit->type == FIGHTER || unit->type == KNIGHT){
        if(!existLeader(unit->y, unit->x)){
          node->troopsId = unit->id;
          unit->townId = node;
          unit->troopsCount = 1;

          if(attackCount == 0){
            unit->troopsLimit = 30;
          }else{
            unit->troopsLimit = 1;
          }
          attackCount += 1;
          //fprintf(stderr,"attack count = %d\n", attackCount);

          return LEADER;
        }else{
          return COMBATANT;
        }
      }else{
        return unit->type;
      }
    }

    /*
     * 行動の優先順位を決める
     */
    int directUnitMovePriority(Unit *unit){
      return 1000 * movePriority[unit->role] - calcManhattanDist(unit->y, unit->x, 79, 79);
    }

    /*
     * 採取モードに移行するかどうかの確認
     */
    bool pickModeCheck(Unit *unit){
      set<int>::iterator it = resourceNodeList.begin();

      while(it != resourceNodeList.end()){
        int y = (*it)/WIDTH;
        int x = (*it)%WIDTH;
        int dist = calcManhattanDist(unit->y, unit->x, y, x);
        Node *node = &gameStage.field[y][x];

        if(!node->rockon && dist <= 10 && checkMinDist(y, x, dist) && !isDie(unit, y, x)){
          node->rockon = true;
          unit->resourceY = y;
          unit->resourceX = x;
          return true;
        }

        it++;
      }

      return false;
    }

    /*
     * 一番距離が近いかの確認
     *       y: 調査したいノードのy座標
     *       x: 調査したいノードのx座標
     * minDist: 現在の最短(調べたいノードとユニットの現在の距離)
     */
    bool checkMinDist(int y, int x, int minDist){
      set<int>::iterator it = myActiveUnitList.begin();

      while(it != myActiveUnitList.end()){
        Unit *unit = &unitList[(*it)];
        int dist = calcManhattanDist(unit->y, unit->x, y, x);

        // 他に最短距離なユニットがいる場合はfalseを返す
        if(minDist > dist && unit->resourceY == UNDEFINED && unit->resourceX == UNDEFINED) return false;

        it++;
      }

      return true;
    }

    /*
     * 自軍の生存確認
     * ユニットのtimestampが更新されていない場合は前のターンで的に倒されたので、
     * リストから排除する。
     */
    void myUnitSurvivalCheck(){
      set<int> tempList = myActiveUnitList;
      set<int>::iterator it = tempList.begin();

      while(it != tempList.end()){
        Unit *unit = &unitList[*it];

        if(unit->timestamp != turn){
          removeMyUnit(unit);
        }else{
          updateNodeTimestamp(unit->y, unit->x);
        }

        it++;
      }
    }

    /*
     * 敵軍の生存確認
     */
    void enemyUnitSurvivalCheck(){
      set<int> tempList = enemyActiveUnitList;
      set<int>::iterator it = tempList.begin();

      while(it != tempList.end()){
        Unit *enemy = &unitList[*it];
        Node *node = getNode(enemy->y, enemy->x);

        if(enemy->timestamp != turn && node->timestamp == turn){
          removeEnemyUnit(enemy);
        }else if(enemy->timestamp == turn){
          checkEnemyMark(enemy);
        }

        it++;
      }
    }

    /*
     * 敵の城から攻撃をうけたかどうかの確認
     */
    void enemyCastelAttackCheck(){
      set<int>::iterator it = myActiveUnitList.begin();

      while(it != myActiveUnitList.end()){
        Unit *unit = &unitList[*it];

        if((unit->mode == SEARCH || unit->mode == DESTROY) && underAttack(unit) && isCastelDamage(unit)){
          //fprintf(stderr,"turn = %d, id = %d, y = %d, x = %d, beforeHp = %d, hp = %d\n", turn, unit->id, unit->y, unit->x, unit->beforeHp, unit->hp);
          unit->castelAttackCount += 1;

          if(!gameStage.castelAttack){
            gameStage.castelAttack = true;
            setCastelPointList(unit->y,unit->x);

            if(gameStage.enemyCastelPointList.size() == 0){
              //fprintf(stderr,"No castel point\n");
              queue<int> que;
              gameStage.enemyCastelPointList = que;
              gameStage.castelAttack = false;
            }else{
              int id = gameStage.enemyCastelPointList.front(); gameStage.enemyCastelPointList.pop();
              int y = id / WIDTH;
              int x = id % WIDTH;

              gameStage.targetY = y;
              gameStage.targetX = x;
            }
          }
        }else{
          unit->castelAttackCount = 0;
        }

        it++;
      }
    }

    /*
     * 敵から攻撃を受けたかどうかの確認
     */
    bool underAttack(Unit *unit){
      return unit->beforeHp > unit->hp;
    }

    /*
     * 評価値の計算
     */
    int calcEvaluation(Unit *unit, int operation){
      int destDist = (unit->mode == SEARCH)? calcManhattanDist(unit->y, unit->x, unit->destY, unit->destX) : 0;
      Node *node = &gameStage.field[unit->y][unit->x];
      int stamp = gameStage.field[unit->y][unit->x].stamp;

      switch(unit->type){
        case WORKER:
          switch(unit->mode){
            case SEARCH:
              if(operation == NO_MOVE){
                return MIN_VALUE;
              }else if(operation == CREATE_BASE && gameStage.baseCount == 0 && (calcManhattanDist(unit->y, unit->x, 99, 99) <= 70 || gameStage.castelAttack || gameStage.incomeResource >= 70)){
                return 100000;
              }else{
                if(isDie(unit, unit->y, unit->x)){
                  return -10000;
                }else if(turn <= 10){
                  return 100 * myResourceCount + 2 * gameStage.openedNodeCount - 3 * destDist - 3 * stamp - node->cost + 10 * aroundMyUnitDist(unit);
                }else{
                  return 100 * myResourceCount + 2 * gameStage.openedNodeCount - 3 * destDist - 2 * stamp - node->cost - max(0, calcReceivedCombatDamage(unit)-300)/10;
                }
              }
              break;
            case PICKING:
              return calcPickingEvaluation(unit, operation);
              break;
            default:
              break;
          }
          break;
        case VILLAGE:
          switch(unit->mode){
            case NONE:
              return calcNoneVillageEvaluation(unit, operation);
              break;
          }
          break;
        case CASTEL:
          switch(unit->mode){
            case NONE:
              return calcNoneCastelEvaluation(unit, operation);
              break;
          }
        case BASE:
          switch(unit->mode){
            case NONE:
              return calcNoneBaseEvaluation(unit, operation);
              break;
          }
          break;
        case KNIGHT:
          if(unit->role == LEADER){
            return calcLeaderEvaluation(unit, operation);
          }else{
            return calcCombatEvaluation(unit, operation);
          }
          break;
        case FIGHTER:
          if(unit->role == LEADER){
            return calcLeaderEvaluation(unit, operation);
          }else{
            return calcCombatEvaluation(unit, operation);
          }
          break;
        case ASSASIN:
          if(unit->role == LEADER){
            return calcLeaderEvaluation(unit, operation);
          }else{
            return calcCombatEvaluation(unit, operation);
          }
          break;
        default:
          break;
      }

      return 0;
    }

    /*
     * PICKING状態での評価値
     * 資源マスにいない状態では資源マスを目指すように
     */
    int calcPickingEvaluation(Unit *unit, int operation){
      Node *node = &gameStage.field[unit->y][unit->x];
      int dist = calcManhattanDist(unit->y, unit->x, 50, 50);

      if(node->resource && unit->resourceY == unit->y && unit->resourceX == unit->x){
        //fprintf(stderr,"Base Count = %d\n", node->myUnitCount[BASE]);
        if(gameStage.gameSituation == WARNING && operation == CREATE_BASE && node->myUnitCount[BASE] == 1){
          return -10000;
        }else if(operation == CREATE_BASE && dist <= 80 && myResourceCount >= 1000 && node->myUnitCount[BASE] == 1){
          return -1000;
        }else if(operation == CREATE_VILLAGE && node->myUnitCount[VILLAGE] == 1){
          return 100;
        }else{
          return 10 * node->myUnitCount[WORKER];
        }
      }else{
        if(isDie(unit, unit->y, unit->x)){
          unit->mode = SEARCH;
          unit->resourceY = UNDEFINED;
          unit->resourceX = UNDEFINED;
          return -10000;
        }else{
          return -calcManhattanDist(unit->y, unit->x, unit->resourceY, unit->resourceX);
        }
      }
    }

    /*
     * 村が動いていない時の評価値
     */
    int calcNoneVillageEvaluation(Unit *village, int operation){
      int centerDist = calcManhattanDist(village->y, village->x, 50, 50);
      int income = gameStage.incomeResource;
      Node *node = &gameStage.field[village->y][village->x];

      if(operation == CREATE_WORKER && node->myUnitCount[WORKER] <= 5 && village->createWorkerCount <= 7){
        return 100;
      }else if(operation != CREATE_WORKER && gameStage.gameSituation == WARNING && village->createWorkerCount >= 5){
        return 1000;
      }else if(operation == CREATE_WORKER && myResourceCount >= 40 && centerDist <= 60 && village->createWorkerCount <= 7){
        return 110;
      }else if(operation != CREATE_WORKER){
        return 10;
      }else{
        return 0;
      }
    }

    /*
     * 城が動いていない時の評価値
     */
    int calcNoneCastelEvaluation(Unit *castel, int operation){
      // 序盤でどれだけワーカーの数を増やすか
      if(operation == CREATE_WORKER && turn <= 12){
        return 100;
      }else{
        return 0;
      }
    }

    /*
     * 拠点が動いていない時の評価値
     */
    int calcNoneBaseEvaluation(Unit *base, int operation){
      if(operation == CREATE_ASSASIN){
        return 100;
      }else{
        return 0;
      }
    }

    /*
     * リーダ時の行動パターン
     */
    int calcLeaderEvaluation(Unit *unit, int operation){
      Node *node = getNode(unit->y, unit->x);

      switch(unit->mode){
        case STAY:
          if(operation != NO_MOVE){
            return -10000;
          }else{
            return 0;
          }
          break;
        case DESTROY:
          if(gameStage.gameSituation == ONRUSH){
            if(node->enemyAttackCount[KNIGHT] + node->enemyAttackCount[FIGHTER] + node->enemyAttackCount[ASSASIN] >= 20){
              if(operation == NO_MOVE){
                return 100 - abs(2-calcManhattanDist(unit->y, unit->x, enemyCastelCoordY, enemyCastelCoordX));
              }else{
                return -100 - abs(2-calcManhattanDist(unit->y, unit->x, enemyCastelCoordY, enemyCastelCoordX));
              }
            }else{
              return -1 * abs(2-calcManhattanDist(unit->y, unit->x, enemyCastelCoordY, enemyCastelCoordX));
            }
          }else if(!gameStage.castelAttack){
            return -2 * calcManhattanDist(unit->y, unit->x, gameStage.targetY, gameStage.targetX) + calcManhattanDist(unit->y, unit->x, 0, 0) - gameStage.field[unit->y][unit->x].stamp;
          }else{
            return -1 * calcManhattanDist(unit->y, unit->x, gameStage.targetY, gameStage.targetX);
          }
          break;
        default:
          if(gameStage.gameSituation == ONRUSH){
            return -1 * calcManhattanDist(unit->y, unit->x, enemyCastelCoordY, enemyCastelCoordX);
          }else if(gameStage.field[unit->y][unit->x].myUnitCount[unit->type] <= 4){
            return -1 * calcManhattanDist(unit->y, unit->x, unit->destY, unit->destX);
          }else if(gameStage.gameSituation == ONRUSH){
            return -1 * calcManhattanDist(unit->y, unit->x, enemyCastelCoordY, enemyCastelCoordX);
          }else{
            return -1 * calcManhattanDist(unit->y, unit->x, 0, 0);
          }
          break;
      }

      return -100;
    }

    /*
     * 戦闘員の行動評価関数
     */
    int calcCombatEvaluation(Unit *unit, int operation){
      Unit *leader = &unitList[unit->leaderId];
      Node *node = &gameStage.field[unit->y][unit->x];
      int limit = 10;

      switch(unit->mode){
        case STAY:
          if(gameStage.field[leader->y][leader->x].myUnitCount[unit->type] <= limit){
            return -100 * calcManhattanDist(unit->y, unit->x, leader->y, leader->x);
          }else{
            return -100 * abs(1-calcManhattanDist(unit->y, unit->x, leader->y, leader->x)) - 10 * node->myUnitCount[unit->type];
          }
          break;
        case DESTROY:
          if(leader->mode == DESTROY){
            if(gameStage.gameSituation == ONRUSH){
              if(gameStage.field[unit->y][unit->x].myUnitCount[unit->type] <= limit && unit->y == leader->y && unit->x == leader->x){
                return 100;
              }else if(gameStage.field[unit->y][unit->x].myUnitCount[unit->type] <= 10){
                return -200 * abs(1-calcManhattanDist(unit->y, unit->x, leader->y, leader->x)) - 10 * node->myUnitCount[unit->type];
              }else{
                return -200 * abs(1-calcManhattanDist(unit->y, unit->x, leader->y, leader->x)) - 10 * node->myUnitCount[unit->type];
              }
            }else{
              if(gameStage.field[leader->y][leader->x].myUnitCount[unit->type] <= limit && unit->y == leader->y && unit->x == leader->x){
                return 100;
              }else{
                return -200 * abs(1-calcManhattanDist(unit->y, unit->x, leader->y, leader->x)) - 10 * node->myUnitCount[unit->type];
                return -1 * abs(1-calcManhattanDist(unit->y, unit->x, leader->y, leader->x)) - 100;
              }
            }
          }else{
            return -1 * calcManhattanDist(unit->y, unit->x, leader->y, leader->x);
          }
          break;
      }

      return -1000;
    }

    /*
     * 自軍のユニットが生きているかどうかの確認
     */
    bool isAlive(int unitId){
      return (myActiveUnitList.find(unitId) != myActiveUnitList.end());
    }

    /*
     * 自軍ユニットとの距離
     */
    int aroundMyUnitDist(Unit *unit){
      int minDist = 9999;
      priority_queue< Coord, vector<Coord>, greater<Coord>  > que;

      set<int>::iterator it = myActiveUnitList.begin();

      while(it != myActiveUnitList.end()){
        Unit *other = &unitList[*it];

        if(unit->id == other->id){
          it++;
          continue;
        }

        if(other->movable){
          minDist = min(minDist, calcManhattanDist(unit->y, unit->x, other->y, other->x));
        }

        it++;
      }

      return minDist;
    }

    /*
     * 敵の攻撃を受ける可能性があるマスにチェックを付ける
     */
    void checkEnemyMark(Unit *enemy){

      map<int, bool> checkList;
      queue<cell> que;
      que.push(cell(Coord(enemy->y, enemy->x), 0));

      while(!que.empty()){
        cell c = que.front(); que.pop();

        Coord coord = c.first;
        int dist = c.second;

        if(checkList[coord.y*WIDTH+coord.x] || dist > enemy->attackRange) continue;
        checkList[coord.y*WIDTH+coord.x] = true;

        Node *node = getNode(coord.y, coord.x);
        node->enemyAttackCount[enemy->type] += 1;

        for(int i = 1; i < 5; i++){
          int ny = coord.y + dy[i];
          int nx = coord.x + dx[i];

          if(!isWall(ny,nx)) que.push(cell(Coord(ny,nx), dist+1));
        }
      }
    }

    /*
     * 敵から受けるかもしれないダメージを計算する
     */
    int calcReceivedCombatDamage(Unit *unit){
      int damage = 0;
      Node *node = getNode(unit->y, unit->x);

      for(int enemyType = 0; enemyType < UNIT_MAX; enemyType++){
        damage += DAMAGE_TABLE[enemyType][unit->type] * node->enemyAttackCount[enemyType];
      }

      return damage;
    }

    /*
     * ノードを取得する
     */
    Node* getNode(int y, int x){
      return &gameStage.field[y][x];
    }

    /*
     * 探索時の基本的なコストを付ける
     * - 壁から3マスは移動しなくても探索可能
     */
    void checkCost(){
      for(int y = 0; y < HEIGHT; y++){
        for(int x = 0; x < WIDTH; x++){
          Node *node = &gameStage.field[y][x];
          int dist = calcNearWallDistance(y,x);

          if(dist <= 3){
            node->cost = (4-dist) * 4;
          }
        }
      }

      map<int, bool> checkList;
      queue<cell> que;
      /*
      que.push(cell(Coord(0, 0), 10));
      que.push(cell(Coord(0, 99), 20));
      que.push(cell(Coord(99, 0), 20));
      que.push(cell(Coord(99, 99), 10));
      */

      while(!que.empty()){
        cell c = que.front(); que.pop();

        Coord coord = c.first;
        int cost = c.second;

        if(checkList[coord.y*WIDTH+coord.x] || cost <= 0) continue;
        checkList[coord.y*WIDTH+coord.x] = true;

        Node *node = &gameStage.field[coord.y][coord.x];
        node->cost += cost;

        for(int i = 1; i < 5; i++){
          int ny = coord.y + dy[i];
          int nx = coord.x + dx[i];

          if(!isWall(ny,nx)) que.push(cell(Coord(ny,nx), cost-1));
        }
      }
    }

    /*
     * 敵の城が無いことをチェックする
     */
    void checkNoEnemyCastel(int y, int x){
      map<int, bool> checkList;
      queue<cell> que;
      que.push(cell(Coord(y, x), 0));

      while(!que.empty()){
        cell c = que.front(); que.pop();

        Coord coord = c.first;
        int dist = c.second;

        if(checkList[coord.y*WIDTH+coord.x] || dist > 10) continue;
        if(calcManhattanDist(coord.y, coord.x, 99 , 99) > 50) continue;
        checkList[coord.y*WIDTH+coord.x] = true;

        Node *node = &gameStage.field[coord.y][coord.x];
        node->enemyCastel = false;

        for(int i = 1; i < 5; i++){
          int ny = coord.y + dy[i];
          int nx = coord.x + dx[i];

          if(!isWall(ny,nx)) que.push(cell(Coord(ny,nx), dist+1));
        }
      }
    }

    /*
     * 近くの壁までの距離を測る
     */
    int calcNearWallDistance(int y, int x){
      int up = y;
      int down = (HEIGHT-1) - y;
      int left = x;
      int right = (WIDTH-1) - x;

      return min(min(up,down), min(left,right));
    }

    /*
     * 敵の城からのダメージを受けたかどうかの判定
     */
    bool isCastelDamage(Unit *unit){
      int currentHp = unit->beforeHp;

      set<int>::iterator it = enemyActiveUnitList.begin();
      if(unit->timestamp != turn) return false;

      if(turn == 184){
        //fprintf(stderr,"id = %d, currentHp = %d, hp = %d, enemyCount = %lu\n", unit->id, currentHp, unit->hp, enemyActiveUnitList.size());
      }

      while(it != enemyActiveUnitList.end()){
        Unit *enemy = &unitList[*it];
        int dist = calcManhattanDist(unit->y, unit->x, enemy->y, enemy->x);
        if(turn == 184){
          //fprintf(stderr,"id = %d, y = %d, x = %d, attackRange = %d\n", enemy->id, enemy->y, enemy->x, enemy->attackRange);
        }

        if(dist <= unitAttackRange[enemy->type]){
          int k = countMyUnit(enemy->y, enemy->x, unitAttackRange[enemy->type]);
          if(enemy->type == VILLAGE || enemy->type == BASE) return false;
          //fprintf(stderr,"attack: k = %d\n", k);
          if(k > 0){
            if(turn == 184){
              //fprintf(stderr,"attack damage = %d, enemyId = %d\n", DAMAGE_TABLE[enemy->type][unit->type] / k, enemy->id);
            }
            currentHp -= DAMAGE_TABLE[enemy->type][unit->type] / k;
          }
        }

        it++;
      }

      if(turn == 184){
        //fprintf(stderr,"id = %d, currentHp = %d, hp = %d\n", unit->id, currentHp, unit->hp);
      }

      return (currentHp != unit->hp);
    }

    /*
     * 死ぬかどうかの確認
     */
    bool isDie(Unit *unit, int ypos, int xpos){
      int currentHp = unit->hp;
      set<int>::iterator it = enemyActiveUnitList.begin();

      while(it != enemyActiveUnitList.end()){
        Unit *enemy = &unitList[*it];
        int dist = calcManhattanDist(ypos, xpos, enemy->y, enemy->x);

        if(dist <= unitAttackRange[enemy->type]){
          int k = countMyUnit(enemy->y, enemy->x, unitAttackRange[enemy->type]);
          if(k > 0){
            currentHp -= DAMAGE_TABLE[enemy->type][unit->type] / k;
          }
        }

        it++;
      }

      return currentHp < unit->hp * 0.7;
    }

    /*
     * 敵の城の候補地を選出する(初めてダメージを受けたマスから範囲10のマスのどこか)
     * 1回の試合で1回しか呼ばれない
     */
    void setCastelPointList(int ypos, int xpos){
      //fprintf(stderr,"setCastelPointList: turn = %d, ypos = %d, xpos = %d\n", turn, ypos, xpos);
      //fprintf(stderr,"check = %d\n", gameStage.field[90][72].enemyCastel);

      for(int y = 0; y < HEIGHT; y++){
        for(int x = 0; x < WIDTH; x++){
          int dist = calcManhattanDist(ypos, xpos, y, x);
          Node *node = &gameStage.field[y][x];

          if(dist == 10 && node->enemyCastel && isCastelPoint(y,x)){
            //fprintf(stderr,"list y = %d, x = %d\n",y, x);
            gameStage.enemyCastelPointList.push(y*WIDTH+x);
          }
        }
      }
    }

    /*
     * そこが敵の城かどうかを判定する
     */
    bool isCastelPoint(int y, int x){
      map<int, bool> checkList;
      queue<cell> que;
      que.push(cell(Coord(y, x), 0));

      while(!que.empty()){
        cell c = que.front(); que.pop();

        Coord coord = c.first;
        int dist = c.second;

        if(checkList[coord.y*WIDTH+coord.x] || dist > 4) continue;
        checkList[coord.y*WIDTH+coord.x] = true;

        Node *node = &gameStage.field[coord.y][coord.x];

        if(node->searched || calcManhattanDist(coord.y, coord.x, 99 , 99) > 40) return false;

        for(int i = 1; i < 5; i++){
          int ny = coord.y + dy[i];
          int nx = coord.x + dx[i];

          if(!isWall(ny,nx)) que.push(cell(Coord(ny,nx), dist+1));
        }
      }

      return true;
    }

    /*
     * 範囲内にいる自軍の数を数える
     */
    int countMyUnit(int y, int x, int range){
      map<int, bool> checkList;
      queue<cell> que;
      int unitCount = 0;
      que.push(cell(Coord(y, x), 0));

      while(!que.empty()){
        cell c = que.front(); que.pop();

        Coord coord = c.first;
        int dist = c.second;

        if(checkList[coord.y*WIDTH+coord.x] || dist > range) continue;
        checkList[coord.y*WIDTH+coord.x] = true;

        Node *node = &gameStage.field[coord.y][coord.x];

        int cnt = node->myUnits.size();
        unitCount += min(10, cnt);

        for(int i = 1; i < 5; i++){
          int ny = coord.y + dy[i];
          int nx = coord.x + dx[i];

          if(!isWall(ny,nx)) que.push(cell(Coord(ny,nx),dist+1));
        }
      }

      return unitCount;
    }

    /*
     * マークを付ける
     */
    void checkMark(int ypos, int xpos){
      int cost = 4;
      map<int, bool> checkList;
      queue<cell> que;
      que.push(cell(Coord(ypos, xpos), cost));

      while(!que.empty()){
        cell c = que.front(); que.pop(); 
        Coord coord = c.first;
        int cost = c.second;

        if(checkList[coord.y*WIDTH+coord.x] || cost < 0) continue;
        checkList[coord.y*WIDTH+coord.x] = true;

        gameStage.field[coord.y][coord.x].markCount += 1;
        //gameStage.field[coord.y][coord.x].cost -= 1;

        for(int i = 1; i < 5; i++){
          int ny = coord.y + dy[i];
          int nx = coord.x + dx[i];
          if(!isWall(ny,nx)) que.push(cell(Coord(ny, nx), cost-1));
        }
      }
    }

    /*
     * マークを外す
     */
    void uncheckMark(int ypos, int xpos){
      int cost = 8;
      map<int, bool> checkList;
      queue<cell> que;
      que.push(cell(Coord(ypos, xpos), cost));

      while(!que.empty()){
        cell c = que.front(); que.pop(); 
        Coord coord = c.first;
        int cost = c.second;

        if(checkList[coord.y*WIDTH+coord.x] || cost < 0) continue;
        checkList[coord.y*WIDTH+coord.x] = true;

        gameStage.field[coord.y][coord.x].markCount -= 1;
        //gameStage.field[coord.y][coord.x].cost -= 1;

        for(int i = 1; i < 5; i++){
          int ny = coord.y + dy[i];
          int nx = coord.x + dx[i];
          if(!isWall(ny,nx)) que.push(cell(Coord(ny, nx), cost-1));
        }
      }
    }

    /*
     *
     */
    void checkStamp(int ypos, int xpos, int eyeRange){
      for(int y = max(0, ypos-eyeRange); y <= min(HEIGHT-1, ypos+eyeRange); y++){
        int diff = 2*abs(ypos-y)/2;

        for(int x = max(0, xpos-eyeRange+diff); x <= min(WIDTH-1, xpos+eyeRange-diff); x++){
          if(isWall(y,x)) continue;

          gameStage.field[y][x].stamp += 1;
        }
      }
    }

    /*
     * 現在確保出来ている視界の数を調べる
     */
    int checkVisibleCount(){
      int visibleNodeCount = 0;

      for(int y = 0; y < HEIGHT; y++){
        for(int x = 0; x < WIDTH; x++){
          if(gameStage.field[y][x].seenCount > 0){
            visibleNodeCount += 1;
          }
        }
      }

      return visibleNodeCount;
    }

    /*
     * fieldの初期化を行う
     */
    void clearField(){
      for(int y = 0; y < HEIGHT; y++){
        for(int x = 0; x < WIDTH; x++){
          Node *node = &gameStage.field[y][x];

          node->seenCount = 0;
          if(turn % 10 == 0){
            node->markCount = 0;
          }
          node->seenMembers.clear();
          node->myUnits.clear();
          node->opened = false;
          if(turn % 4 == 0){
            node->stamp = 0;
          }

          memset(node->myUnitCount, 0, sizeof(node->myUnitCount));
          memset(node->enemyUnitCount, 0, sizeof(node->enemyUnitCount));
          memset(node->enemyAttackCount, 0, sizeof(node->enemyAttackCount));
        }
      }
    }

    /*
     * ゲームの実行
     */
    void run(){
      init();

      // 残り時間(ms)が取得出来なくなるまで回し続ける
      while(cin >> remainingTime){
        //fprintf(stderr, "Remaing time is %dms\n", remainingTime);

        // フィールドのクリア
        clearField();

        // 各ターンで行う処理(主に入力の処理)
        eachTurnProc();

        // 敵の城から攻撃を受けたかどうかのチェック
        enemyCastelAttackCheck();

        // 自軍の生存確認
        myUnitSurvivalCheck();

        // 敵軍の生存確認
        enemyUnitSurvivalCheck();

        // 試合状況更新
        updateGameSituation();

        // 自軍の各ユニットのモード変更を行う
        updateUnitMode();

        vector<Operation> operationList;
        // 行動フェーズ
        operationList = actionPhase();

        // 最終的な出力
        finalOperation(operationList);
      }
    }

    /*
     * 最終指示(このターンの最終的な行動を出力)
     */
    void finalOperation(vector<Operation> &operationList){
      int size = operationList.size();
      if(gameStage.gameSituation != ONRUSH){
        //fprintf(stderr,"finalOperation: size = %d\n", size);
        //fprintf(stderr,"openedNodeCount = %d\n", gameStage.openedNodeCount);
        //fprintf(stderr,"visibleNodeCount = %d\n", gameStage.visibleNodeCount);
      }

      printf("%d\n", size);
      for(int i = 0; i < size; i++){
        Operation ope = operationList[i];
        printf("%d %c\n", ope.unitId, instruction[ope.operation]);
      }
    }

    /*
     * 視界をチェックする(探索済みのマスを増やす)
     *   unitId: ユニットID
     *     ypos: y座標
     *     xpos: x座標
     * eyeRange: 視界の広さ
     */
    void checkNode(int unitId, int ypos, int xpos, int eyeRange){
      for(int y = max(0, ypos-eyeRange); y <= min(HEIGHT-1, ypos+eyeRange); y++){
        int diff = 2*abs(ypos-y)/2;

        for(int x = max(0, xpos-eyeRange+diff); x <= min(WIDTH-1, xpos+eyeRange-diff); x++){
          if(isWall(y,x)) continue;

          gameStage.field[y][x].seenMembers.insert(unitId);
          gameStage.field[y][x].seenCount += 1;

          if(!gameStage.field[y][x].searched){
            gameStage.searchedNodeCount += 1;
            gameStage.field[y][x].searched = true;
          }

          if(!gameStage.field[y][x].opened){
            gameStage.visibleNodeCount += 1;
            gameStage.field[y][x].opened = true;
          }
        }
      }

      //fprintf(stderr,"searchedNodeCount = %d\n", gameStage.searchedNodeCount);
    }

    /*
     * 視界のアンチェックを行う
     */
    void uncheckNode(int unitId, int ypos, int xpos, int eyeRange){
      for(int y = max(0, ypos-eyeRange); y <= min(HEIGHT-1, ypos+eyeRange); y++){
        int diff = 2*abs(ypos-y)/2;

        for(int x = max(0, xpos-eyeRange+diff); x <= min(WIDTH-1, xpos+eyeRange-diff); x++){
          if(isWall(y,x)) continue;

          gameStage.field[y][x].seenMembers.erase(unitId);
        }
      }
    }

    /*
     * 視界をオープンする(仮想的)
     *      ypos: y座標
     *      xpos: x座標
     *  eyeRange: 視界の広さ
     */
    void openNode(int ypos, int xpos, int eyeRange){
      for(int y = max(0, ypos-eyeRange); y <= min(HEIGHT-1, ypos+eyeRange); y++){
        int diff = 2*abs(ypos-y)/2;

        for(int x = max(0, xpos-eyeRange+diff); x <= min(WIDTH-1, xpos+eyeRange-diff); x++){
          if(isWall(y,x)) continue;

          gameStage.field[y][x].seenCount += 1;

          gameStage.openedNodeCount += !gameStage.field[y][x].searched;
          gameStage.visibleNodeCount += !gameStage.field[y][x].opened;

          gameStage.field[y][x].opened = true;
        }
      }
    }

    /*
     * 視界をクローズする(openNodeのrollback用)
     *      ypos: y座標
     *      xpos: x座標
     *  eyeRange: 視界の広さ
     */
    void closeNode(int ypos, int xpos, int eyeRange){
      for(int y = max(0, ypos-eyeRange); y <= min(HEIGHT-1, ypos+eyeRange); y++){
        int diff = 2*abs(ypos-y)/2;

        for(int x = max(0, xpos-eyeRange+diff); x <= min(WIDTH-1, xpos+eyeRange-diff); x++){
          if(isWall(y,x)) continue;
          gameStage.field[y][x].seenCount -= 1;

          bool opened = (gameStage.field[y][x].seenCount > 0);

          gameStage.openedNodeCount -= !gameStage.field[y][x].searched;
          gameStage.visibleNodeCount -= gameStage.field[y][x].opened ^ opened;
          gameStage.field[y][x].opened = opened;
        }
      }
    }


    /*
     * ユニットが行動を起こす
     * 行動が成功した場合はtrue、失敗した場合は場合はfalseを返す
     *   unit: ユニット
     *   type: 行動の種別
     *  final: 確定した行動の場合はtrue
     */
    bool unitAction(Unit *unit, int operationType, bool final = false){
      switch(operationType){
        case MOVE_UP:
          if(canMove(unit->y, unit->x, MOVE_UP)){
            closeNode(unit->y, unit->x, unit->eyeRange);

            moveUp(unit);

            if(final){
              checkNode(unit->id, unit->y, unit->x, unit->eyeRange);
            }else{
              openNode(unit->y, unit->x, unit->eyeRange);
            }
          }else{
            return false;
          }
          break;
        case MOVE_DOWN:
          if(canMove(unit->y, unit->x, MOVE_DOWN)){
            closeNode(unit->y, unit->x, unit->eyeRange);

            moveDown(unit);

            if(final){
              checkNode(unit->id, unit->y, unit->x, unit->eyeRange);
            }else{
              openNode(unit->y, unit->x, unit->eyeRange);
            }
          }else{
            return false;
          }
          break;
        case MOVE_LEFT:
          if(canMove(unit->y, unit->x, MOVE_LEFT)){
            closeNode(unit->y, unit->x, unit->eyeRange);

            moveLeft(unit);

            if(final){
              checkNode(unit->id, unit->y, unit->x, unit->eyeRange);
            }else{
              openNode(unit->y, unit->x, unit->eyeRange);
            }
          }else{
            return false;
          }
          break;
        case MOVE_RIGHT:
          if(canMove(unit->y, unit->x, MOVE_RIGHT)){
            closeNode(unit->y, unit->x, unit->eyeRange);

            moveRight(unit);

            if(final){
              checkNode(unit->id, unit->y, unit->x, unit->eyeRange);
            }else{
              openNode(unit->y, unit->x, unit->eyeRange);
            }
          }else{
            return false;
          }
          break;
        case CREATE_WORKER:
          if(canBuild(unit->type, WORKER)){
            createUnit(unit->y, unit->x, WORKER);

            if(final){
              unit->createWorkerCount += 1;
            }
          }else{
            return false;
          }
          break;
        case CREATE_KNIGHT:
          if(canBuild(unit->type, KNIGHT)){
            createUnit(unit->y, unit->x, KNIGHT);

            if(final){
              unit->createKnightCount += 1;
            }
          }else{
            return false;
          }
          break;
        case CREATE_FIGHTER:
          if(canBuild(unit->type, FIGHTER)){
            createUnit(unit->y, unit->x, FIGHTER);
          }else{
            return false;
          }
          break;
        case CREATE_ASSASIN:
          if(canBuild(unit->type, ASSASIN)){
            createUnit(unit->y, unit->x, ASSASIN);
          }else{
            return false;
          }
          break;
        case CREATE_VILLAGE:
          if(canBuild(unit->type, VILLAGE)){
            createUnit(unit->y, unit->x, VILLAGE);
          }else{
            return false;
          }
          break;
        case CREATE_BASE:
          if(canBuild(unit->type, BASE)){
            createUnit(unit->y, unit->x, BASE);

            if(final){
              gameStage.baseCount += 1;
            }
          }else{
            return false;
          }
          break;
        default:
          noMove();
          break;
      }

      return true;
    }

    /*
     * ユニットのアクションの取消を行う
     * unitId: ユニットID
     *   type: アクションの種類
     */
    void rollbackAction(Unit *unit, int type){
      switch(type){
        case MOVE_UP:
          closeNode(unit->y, unit->x, unit->eyeRange);
          moveDown(unit);
          openNode(unit->y, unit->x, unit->eyeRange);
          break;
        case MOVE_DOWN:
          closeNode(unit->y, unit->x, unit->eyeRange);
          moveUp(unit);
          openNode(unit->y, unit->x, unit->eyeRange);
          break;
        case MOVE_LEFT:
          closeNode(unit->y, unit->x, unit->eyeRange);
          moveRight(unit);
          openNode(unit->y, unit->x, unit->eyeRange);
          break;
        case MOVE_RIGHT:
          closeNode(unit->y, unit->x, unit->eyeRange);
          moveLeft(unit);
          openNode(unit->y, unit->x, unit->eyeRange);
          break;
        case CREATE_WORKER:
          deleteUnit(unit->y, unit->x, WORKER);
          break;
        case CREATE_KNIGHT:
          deleteUnit(unit->y, unit->x, KNIGHT);
          break;
        case CREATE_FIGHTER:
          deleteUnit(unit->y, unit->x, FIGHTER);
          break;
        case CREATE_ASSASIN:
          deleteUnit(unit->y, unit->x, ASSASIN);
          break;
        case CREATE_VILLAGE:
          deleteUnit(unit->y, unit->x, VILLAGE);
          break;
        case CREATE_BASE:
          deleteUnit(unit->y, unit->x, BASE);
          break;
        default:
          noMove();
          break;
      }
    }

    /*
     * ユニットの数を移動させる
     *        y: y座標
     *        x: x座標
     *   direct: 動く方向
     * unitType: ユニットの種類
     */
    void moveUnitCount(int y, int x, int direct, int unitType){
      gameStage.field[y][x].myUnitCount[unitType] -= 1;
      gameStage.field[y+dy[direct]][x+dx[direct]].myUnitCount[unitType] += 1;
    }

    /*
     * 何も行動しない
     */
    void noMove(){
    }

    /*
     * 上に動く
     */
    void moveUp(Unit *unit){
      moveUnitCount(unit->y, unit->x, MOVE_UP, unit->type);
      unit->y -= 1;
    }

    /*
     * 下に動く
     */
    void moveDown(Unit *unit){
      moveUnitCount(unit->y, unit->x, MOVE_DOWN, unit->type);
      unit->y += 1;
    }

    /*
     * 左に動く
     */
    void moveLeft(Unit *unit){
      moveUnitCount(unit->y, unit->x, MOVE_LEFT, unit->type);
      unit->x -= 1;
    }

    /*
     * 右に動く
     */
    void moveRight(Unit *unit){
      moveUnitCount(unit->y, unit->x, MOVE_RIGHT, unit->type);
      unit->x += 1;
    }

    /*
     * 行動フェーズ
     * 自軍に対して各種行動を選択する
     */
    vector<Operation> actionPhase(){
      set<int>::iterator it = myActiveUnitList.begin();
      vector<Operation> operationList;
      priority_queue<MovePriority, vector<MovePriority>, greater<MovePriority> > prique;

      // 各ユニット毎に処理を行う
      while(it != myActiveUnitList.end()){
        MovePriority mp(*it, directUnitMovePriority(&unitList[*it]));
        prique.push(mp);
        it++;
      }

      while(!prique.empty()){
        MovePriority mp = prique.top(); prique.pop();
        Unit *unit = &unitList[mp.unitId];

        Operation bestOperation = directUnitOperation(unit);

        // 行動なし以外はリストに入れる
        if(bestOperation.operation != NONE){
          operationList.push_back(bestOperation);

          // 確定した行動はそのままにする
          unitAction(unit, bestOperation.operation, REAL);

          // 足跡を付ける(足跡が多い場所はなるべく探索しないように)
          if(unit->mode == SEARCH){
            checkStamp(unit->y, unit->x, unit->eyeRange * 2);
          }

          // SEARCHモードのユニットが目的地に到達した場合は、目的地の座標をリセット
          if(unit->mode == SEARCH && (unit->y == unit->destY && unit->x == unit->destX)){
            resetDestPoint(unit);
          }
        }
      }

      return operationList;
    }

    /*
     * 目的地のリセットを行う
     */
    void resetDestPoint(Unit *unit){
      uncheckMark(unit->destY, unit->destX);
      unit->destY = UNDEFINED;
      unit->destX = UNDEFINED;
    }

    /*
     * 命令を決定する
     */
    Operation directUnitOperation(Unit *unit){
      priority_queue<Operation, vector<Operation>, greater<Operation> > que;
      gameStage.searchedNodeCount = 0;

      //tempGameStage = gameStage;

      for(int operation = 0; operation < OPERATION_MAX; operation++){
        if(!OPERATION_LIST[unit->type][operation]) continue;

        // 行動が成功した時だけ評価を行う
        if(unitAction(unit, operation)){
          Operation ope;
          ope.unitId = unit->id;
          ope.operation = operation;
          ope.evaluation = calcEvaluation(unit, operation);

          // 行動を元に戻す
          rollbackAction(unit, operation);

          que.push(ope);
        }

        //gameStage = tempGameStage;
      }

      return que.top();
    }

    /*
     * 渡された座標のマンハッタン距離を計算
     */
    int calcManhattanDist(int y1, int x1, int y2, int x2){
      return manhattanDist[x1*WIDTH+x2] + manhattanDist[y1*WIDTH+y2];
    }

    /*
     * 渡された座標が壁かどうかを判定する。
     *   y: y座標
     *   x: x座標
     */
    bool isWall(int y, int x){
      return walls[y+1][x+1];
    }

    /*
     * 移動が出来るかどうかのチェックを行う
     *   y: y座標
     *   x: x座標
     */
    bool canMove(int y, int x, int direct){
      int ny = y + dy[direct];
      int nx = x + dx[direct];

      return !isWall(ny,nx);
    }

    /*
     * ユニットの生産が可能かどうか
     *   buildType: 生産したい物
     *   unitTType: ユニットの種類
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

    /*
     * テスト用のコード(ダミーユニットの作成を行う)
     */
    Unit* createDummyUnit(int unitId, int y, int x, int hp, int unitType){
      addMyUnit(unitId, y, x, hp, unitType);
      return &unitList[unitId];
    }

    /*
     * テスト用のコード(ダミーユニットの作成を行う)
     */
    Unit* createDummyEnemyUnit(int unitId, int y, int x, int hp, int unitType){
      addEnemyUnit(unitId, y, x, hp, unitType);
      return &unitList[unitId];
    }
};

/*
 * ここから下はテストコード
 */
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
    fprintf(stderr, "TestCase16:\t%s\n", testCase16()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase17:\t%s\n", testCase17()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase18:\t%s\n", testCase18()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase19:\t%s\n", testCase19()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase20:\t%s\n", testCase20()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase21:\t%s\n", testCase21()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase22:\t%s\n", testCase22()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase23:\t%s\n", testCase23()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase24:\t%s\n", testCase24()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase25:\t%s\n", testCase25()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase26:\t%s\n", testCase26()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase27:\t%s\n", testCase27()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase28:\t%s\n", testCase28()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase29:\t%s\n", testCase29()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase30:\t%s\n", testCase30()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase31:\t%s\n", testCase31()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase32:\t%s\n", testCase32()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase33:\t%s\n", testCase33()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase34:\t%s\n", testCase34()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase35:\t%s\n", testCase35()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase36:\t%s\n", testCase36()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase37:\t%s\n", testCase37()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase38:\t%s\n", testCase38()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase39:\t%s\n", testCase39()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase40:\t%s\n", testCase40()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase41:\t%s\n", testCase41()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase42:\t%s\n", testCase42()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase43:\t%s\n", testCase43()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase44:\t%s\n", testCase44()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase45:\t%s\n", testCase45()? "SUCCESS!" : "FAILED!");
    fprintf(stderr, "TestCase46:\t%s\n", testCase46()? "SUCCESS!" : "FAILED!");
  }

  /*
   * Case1: マンハッタン距離が取得出来ているかどうかの確認
   */
  bool testCase1(){
    if(cv.calcManhattanDist(0,0,1,1) != 2) return false;
    if(cv.calcManhattanDist(0,0,0,0) != 0) return false;
    if(cv.calcManhattanDist(99,99,99,99) != 0) return false;
    if(cv.calcManhattanDist(0,99,99,0) != 198) return false;
    if(cv.calcManhattanDist(3,20,9,19) != 7) return false;
    if(cv.calcManhattanDist(0,0,50,50) != 100) return false;

    return true;
  }

  /*
   * Case2: サンプル入力がしっかりと取れているかどうか
   */
  bool testCase2(){
    if(stageNumber != 0) return false;
    if(turn != 27) return false;
    fprintf(stderr,"myResourceCount = %d\n", myResourceCount);
    if(myResourceCount != 29) return false;
    if(myAllUnitCount != 13) return false;
    if(enemyAllUnitCount != 1) return false;
    if(resourceCount != 1) return false;
    if(myCastelCoordY != 7 || myCastelCoordX != 16) return false;

    return true;
  }

  /*
   * Case3: ステージの初期化が成功しているかどうかの確認
   */
  bool testCase3(){
    unitIdCheckList.clear();

    unitIdCheckList[1] = true;
    if(unitIdCheckList.size() != 1) return false;
    cv.stageInitialize();
    if(unitIdCheckList.size() != 0) return false;
    if(myActiveUnitList.size() != 0) return false;
    if(enemyActiveUnitList.size() != 0) return false;
    if(resourceNodeList.size() != 0) return false;
    if(gameStage.enemyCastelPointList.size() != 0) return false;
    if(gameStage.searchedNodeCount != 0) return false;
    if(gameStage.visibleNodeCount != 0) return false;
    if(gameStage.openedNodeCount != 0) return false;
    if(gameStage.castelAttack) return false;

    for(int y = 0; y < HEIGHT; y++){
      for(int x = 0; x < WIDTH; x++){
        Node *node = &gameStage.field[y][x];

        if(node->seenCount != 0) return false;
        if(node->resource) return false;
      } 
    }

    return true;
  }

  /*
   * Case4: 壁判定がちゃんと出来ているかどうか
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
   * Case5: 移動判定が出来ているかどうか
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
   * Case6: 「上に移動」が出来ているかどうか
   */
  bool testCase6(){
    cv.stageInitialize();

    Unit *unit = cv.createDummyUnit(0, 10, 10, 2000, WORKER);
    int x = unit->x;
    int y = unit->y;
    gameStage.field[y][x].myUnitCount[WORKER] = 1;

    cv.moveUp(unit);

    if(0 != gameStage.field[y][x].myUnitCount[unit->type]) return false;

    return (x == unit->x && y-1 == unit->y);
  }

  /*
   * Case7: 「下に移動」が出来ているかどうか
   */
  bool testCase7(){
    cv.stageInitialize();
    Unit *unit = cv.createDummyUnit(0, 10, 10, 2000, WORKER);
    int x = unit->x;
    int y = unit->y;
    gameStage.field[y][x].myUnitCount[WORKER] = 1;

    cv.moveDown(unit);

    if(0 != gameStage.field[y][x].myUnitCount[unit->type]) return false;

    return (x == unit->x && y+1 == unit->y);
  }

  /*
   * Case8: 「左に移動」が出来ているかどうか
   */
  bool testCase8(){
    cv.stageInitialize();
    Unit *unit = cv.createDummyUnit(0, 10, 10, 2000, WORKER);
    int x = unit->x;
    int y = unit->y;
    gameStage.field[y][x].myUnitCount[WORKER] = 1;

    cv.moveLeft(unit);

    if(0 != gameStage.field[y][x].myUnitCount[unit->type]) return false;

    return (x-1 == unit->x && y == unit->y);
  }

  /*
   * Case9: 「右に移動」が来ているかどうか
   */
  bool testCase9(){
    cv.stageInitialize();
    Unit *unit = cv.createDummyUnit(0, 10, 10, 2000, WORKER);
    int x = unit->x;
    int y = unit->y;
    gameStage.field[y][x].myUnitCount[WORKER] = 1;

    cv.moveRight(unit);

    if(0 != gameStage.field[y][x].myUnitCount[unit->type]) return false;

    return (x+1 == unit->x && y == unit->y);
  }

  /*
   * Case10: 「生産可否判定」が出来ているかどうか
   */
  bool testCase10(){
    cv.stageInitialize();

    Unit *castel  = cv.createDummyUnit(0, 10, 10, 50000, CASTEL);
    Unit *village = cv.createDummyUnit(1, 11, 11, 20000, VILLAGE);
    Unit *base    = cv.createDummyUnit(2, 12, 12, 20000, BASE);
    Unit *worker  = cv.createDummyUnit(3, 13, 13, 2000, WORKER);

    myResourceCount = 19;
    if(cv.canBuild(castel->type, KNIGHT)) return false;
    if(cv.canBuild(village->type, WORKER)) return false;
    if(cv.canBuild(base->type, KNIGHT)) return false;
    if(cv.canBuild(base->type, FIGHTER)) return false;

    myResourceCount = 20;
    if(!cv.canBuild(base->type, KNIGHT)) return false;
    if(cv.canBuild(worker->type, KNIGHT)) return false;
    if(cv.canBuild(village->type, KNIGHT)) return false;

    myResourceCount = 40;
    if(!cv.canBuild(village->type, WORKER)) return false;
    if(!cv.canBuild(castel->type, WORKER)) return false;
    if(!cv.canBuild(base->type, KNIGHT)) return false;
    if(!cv.canBuild(base->type, FIGHTER)) return false;
    if(cv.canBuild(base->type, ASSASIN)) return false;
    if(cv.canBuild(village->type, FIGHTER)) return false;

    myResourceCount = 60;
    if(!cv.canBuild(base->type, ASSASIN)) return false;
    if(cv.canBuild(castel->type, ASSASIN)) return false;
    if(cv.canBuild(worker->type, VILLAGE)) return false;

    myResourceCount = 100;
    if(!cv.canBuild(worker->type, VILLAGE)) return false;
    if(!cv.canBuild(base->type, KNIGHT)) return false;
    if(!cv.canBuild(base->type, FIGHTER)) return false;
    if(!cv.canBuild(base->type, ASSASIN)) return false;
    if(cv.canBuild(worker->type, BASE)) return false;

    myResourceCount = 500;
    if(!cv.canBuild(worker->type, BASE)) return false;
    if(cv.canBuild(village->type, BASE)) return false;
    if(!cv.canBuild(worker->type, BASE)) return false;
    if(cv.canBuild(castel->type, BASE)) return false;

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
    if(gameStage.visibleNodeCount != 41) return false;

    myResourceCount = 20;
    cv.createUnit(1,1,KNIGHT);
    if(myResourceCount != 0) return false;
    if(gameStage.field[1][1].myUnitCount[KNIGHT] != 1) return false;
    if(gameStage.visibleNodeCount != 60) return false;

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
   * Case12: ノードの作成が出来ているかどうか
   */
  bool testCase12(){
    Node node = cv.createNode();

    if(node.opened) return false;
    if(node.myUnitCount[WORKER] != 0) return false;
    if(node.myUnitCount[FIGHTER] != 0) return false;
    if(node.myUnitCount[BASE] != 0) return false;
    if(node.enemyUnitCount[WORKER] != 0) return false;
    if(node.enemyUnitCount[FIGHTER] != 0) return false;
    if(node.enemyUnitCount[BASE] != 0) return false;
    if(node.seenMembers.size() != 0) return false;
    if(node.timestamp != 0) return false;
    if(node.seenCount != 0) return false;
    if(node.cost != 0) return false;
    if(node.stamp != 0) return false;
    if(node.markCount != 0) return false;
    if(node.resource) return false;
    if(node.rockon) return false;
    if(node.searched) return false;

    return true;
  }

  /*
   * Case13: ユニットの追加が出来ているかどうかの確認
   */
  bool testCase13(){
    cv.stageInitialize();

    int unitId = 100;
    cv.addMyUnit(unitId, 10, 10, 1980, WORKER);

    Unit *unit = &unitList[unitId];
    if(unit->type != WORKER) return false;
    if(unit->hp != 1980) return false;
    if(unit->mode != SEARCH) return false;
    if(unit->destY != UNDEFINED) return false;
    if(unit->destX != UNDEFINED) return false;
    if(unit->createWorkerCount != 0) return false;
    if(!unit->movable) return false;
    if(!unitIdCheckList[unitId]) return false;
    if(gameStage.searchedNodeCount != 41) return false;
    if(gameStage.field[10][10].seenMembers.size() != 1) return false;

    unitId = 101;
    cv.addMyUnit(unitId, 50, 50, 20000, VILLAGE);
    if(unitList[unitId].type != VILLAGE) return false;
    if(unitList[unitId].hp != 20000) return false;
    if(unitList[unitId].movable) return false;
    if(gameStage.searchedNodeCount != 262) return false;

    unitId = 102;
    cv.addMyUnit(unitId, 30, 30, 20000, BASE);
    if(unitList[unitId].type != BASE) return false;
    if(unitList[unitId].hp != 20000) return false;
    if(unitList[unitId].movable) return false;

    //if(myActiveUnitList.size() != 3) return false;


    return true;
  }

  /*
   * Case14: ユニットの生存確認が出来ているかどうかの確認
   */
  bool testCase14(){
    int unitId = 100;
    cv.stageInitialize();

    cv.addMyUnit(unitId, 10, 10, 1980, WORKER);
    if(gameStage.visibleNodeCount != 41) return false;
    if(gameStage.openedNodeCount != 0) return false;

    unitId = 101;
    cv.addMyUnit(unitId, 20, 20, 1980, WORKER);
    unitList[unitId].timestamp = -1;
    if(gameStage.visibleNodeCount != 82) return false;
    if(gameStage.openedNodeCount != 0) return false;

    cv.myUnitSurvivalCheck();

    if(myActiveUnitList.size() != 1) return false;
    if(myActiveUnitList.find(100) == myActiveUnitList.end()) return false;
    if(myActiveUnitList.find(101) != myActiveUnitList.end()) return false;

    return true;
  }

  /*
   * Case15: ユニットの削除が出来ているかどうかの確認
   */
  bool testCase15(){
    cv.stageInitialize();

    myResourceCount = 80;
    cv.createUnit(1,1,WORKER);
    cv.createUnit(5,5,WORKER);

    cv.deleteUnit(1,1,WORKER);
    if(myResourceCount != 40) return false;
    if(gameStage.visibleNodeCount != 41) return false;
    if(gameStage.field[1][1].myUnitCount[WORKER] != 0) return false;

    cv.deleteUnit(5,5,WORKER);
    if(myResourceCount != 80) return false;
    if(gameStage.visibleNodeCount != 0) return false;
    if(gameStage.field[5][5].myUnitCount[WORKER] != 0) return false;

    return true;
  }

  /*
   * Case16: ユニットが取れるアクションについて制限が取れている
   */
  bool testCase16(){
    cv.stageInitialize();

    int unitId = 100;
    cv.addMyUnit(unitId, 0, 0, 1980, WORKER);

    Unit *unit = &unitList[unitId];
    myResourceCount = COST_MAX;

    if(cv.unitAction(unit, MOVE_UP)) return false;
    if(!cv.unitAction(unit, MOVE_DOWN)) return false;
    if(cv.unitAction(unit, MOVE_LEFT)) return false;
    if(!cv.unitAction(unit, MOVE_RIGHT)) return false;
    if(cv.unitAction(unit, CREATE_WORKER)) return false;
    if(cv.unitAction(unit, CREATE_KNIGHT)) return false;
    if(cv.unitAction(unit, CREATE_FIGHTER)) return false;
    if(cv.unitAction(unit, CREATE_ASSASIN)) return false;
    if(!cv.unitAction(unit, CREATE_VILLAGE)) return false;
    if(!cv.unitAction(unit, CREATE_BASE)) return false;

    return true;
  }

  /*
   * Case17: ロールバックが出来ているかどうか
   */
  bool testCase17(){
    cv.stageInitialize();

    int unitId = 100;
    cv.addMyUnit(unitId, 10, 10, 1980, WORKER);

    Unit *unit = &unitList[unitId];
    myResourceCount = COST_MAX;

    if(gameStage.searchedNodeCount != 41) return false;
    if(gameStage.visibleNodeCount != 41) return false;

    cv.unitAction(unit, MOVE_UP);
    if(gameStage.visibleNodeCount != 41) return false;

    cv.rollbackAction(unit, MOVE_UP);
    if(gameStage.visibleNodeCount != 41) return false;

    return true;
  }

  /*
   * Case18: ユニットの更新が出来ているかどうか
   */
  bool testCase18(){
    cv.stageInitialize();

    int unitId = 100;
    cv.addMyUnit(unitId, 10, 10, 1980, WORKER);

    if(gameStage.field[10][10].seenMembers.size() != 1) return false;
    if(gameStage.field[10][10].myUnitCount[WORKER] != 1) return false;

    cv.updateMyUnitStatus(unitId, 10, 10, 1980);

    if(gameStage.field[10][10].seenMembers.size() != 1) return false;

    return true;
  }

  /*
   * Case19: ユニットの移動の際に視界の広さが取得出来ているかどうか
   */
  bool testCase19(){
    cv.stageInitialize();

    int unitId = 100;
    cv.addMyUnit(unitId, 10, 10, 1980, WORKER);

    unitId = 101;
    cv.addMyUnit(unitId, 10, 10, 1980, WORKER);

    unitId = 102;
    cv.addMyUnit(unitId, 10, 10, 1980, WORKER);

    if(gameStage.visibleNodeCount != 41) return false;

    cv.unitAction(&unitList[100], MOVE_UP);
    if(gameStage.visibleNodeCount != 50) return false;

    cv.unitAction(&unitList[101], MOVE_DOWN);
    if(gameStage.visibleNodeCount != 59) return false;

    cv.rollbackAction(&unitList[100], MOVE_UP);
    cv.rollbackAction(&unitList[101], MOVE_DOWN);

    if(gameStage.visibleNodeCount != 41) return false;

    return true;
  }

  /*
   * Case20: 確保出来ている視界の数が取得できているかどうか
   */
  bool testCase20(){
    cv.stageInitialize();

    int unitId = 100;
    cv.addMyUnit(unitId, 10, 10, 1980, WORKER);

    if(cv.checkVisibleCount() != 41) return false;

    cv.unitAction(&unitList[unitId], MOVE_UP);
    if(gameStage.visibleNodeCount != 41) return false;

    cv.unitAction(&unitList[unitId], MOVE_DOWN);
    if(gameStage.visibleNodeCount != 41) return false;

    return true;
  }

  /*
   * Case21: 調査予定のマスの数が取得出来ているかどうか
   */
  bool testCase21(){
    cv.stageInitialize();

    int unitId = 100;
    cv.addMyUnit(unitId, 10, 10, 1980, WORKER);

    if(gameStage.searchedNodeCount != 41) return false;
    if(gameStage.visibleNodeCount != 41) return false;
    if(gameStage.openedNodeCount != 0) return false;

    cv.unitAction(&unitList[unitId], MOVE_DOWN);
    if(gameStage.visibleNodeCount != 41) return false;
    if(gameStage.searchedNodeCount != 41) return false;
    if(gameStage.openedNodeCount != 9) return false;

    cv.rollbackAction(&unitList[unitId], MOVE_DOWN);
    if(gameStage.searchedNodeCount != 41) return false;
    if(gameStage.visibleNodeCount != 41) return false;
    if(gameStage.openedNodeCount != 0) return false;

    cv.unitAction(&unitList[unitId], MOVE_DOWN, REAL);

    unitId = 101;
    cv.addMyUnit(unitId,  10, 10, 1980, WORKER);

    cv.unitAction(&unitList[unitId], MOVE_LEFT);
    if(gameStage.visibleNodeCount != 50) return false;
    if(gameStage.openedNodeCount != 5) return false;

    return true;
  }

  /*
   * Case22: 同じノードに何体生産しても値が変化しない
   */
  bool testCase22(){
    cv.stageInitialize();

    int unitId = 100;
    myResourceCount = COST_MAX;
    cv.addMyUnit(unitId, 10, 10, 2000, VILLAGE);

    unitId = 101;
    cv.addMyUnit(unitId, 10, 10, 2000, WORKER);

    if(gameStage.searchedNodeCount != 221) return false;
    if(gameStage.visibleNodeCount != 221) return false;
    if(gameStage.openedNodeCount != 0) return false;

    cv.unitAction(&unitList[unitId], MOVE_RIGHT);
    cv.rollbackAction(&unitList[unitId], MOVE_RIGHT);

    if(gameStage.searchedNodeCount != 221) return false;
    if(gameStage.visibleNodeCount != 221) return false;
    if(gameStage.openedNodeCount != 0) return false;

    return true;
  }

  /*
   * Case23: 採取モードに移行出来ているかどうかの確認
   */
  bool testCase23(){
    cv.stageInitialize();

    myResourceCount = COST_MAX;

    Unit *unitA = cv.createDummyUnit(100, 10, 10, 2000, WORKER);
    Unit *unitB = cv.createDummyUnit(101, 11, 11, 2000, WORKER);
    if(unitA->mode != SEARCH) return false;

    cv.addResourceNode(unitA->y+3, unitA->x+3);
    if(cv.directUnitMode(unitA) == PICKING) return false;
    if(cv.directUnitMode(unitB) != PICKING) return false;

    return true;
  }

  /*
   * Case24: 資源マスの追加ができているかどうか
   */
  bool testCase24(){
    cv.stageInitialize();

    if(resourceNodeList.size() != 0) return false;
    cv.addResourceNode(10, 10);

    if(resourceNodeList.size() != 1) return false;
    cv.addResourceNode(10, 10);
    if(resourceNodeList.size() != 1) return false;

    cv.addResourceNode(10, 20);
    if(resourceNodeList.size() != 2) return false;

    return true;
  }

  /*
   * Case25: 最初のモードがちゃんと決められるか
   */
  bool testCase25(){
    cv.stageInitialize();

    cv.addResourceNode(10, 10);

    Unit *unit = cv.createDummyUnit(100, 10, 10, 2000, WORKER);

    if(unit->resourceY != 10 || unit->resourceX != 10) return false;
    if(unit->mode != PICKING) return false;

    gameStage.field[10][10].myUnitCount[WORKER] = 6;
    unit = cv.createDummyUnit(101, 10, 10, 2000, WORKER);
    if(unit->mode == PICKING) return false;

    return true;
  }

  /*
   * Case26: 行動の優先順位が設定されているかどうか
   */
  bool testCase26(){
    cv.stageInitialize(); 

    Unit *worker = cv.createDummyUnit(100, 10, 10, 2000, WORKER);
    int workerMovePirority = cv.directUnitMovePriority(worker);

    Unit *village = cv.createDummyUnit(101, 11, 11, 2000, VILLAGE);
    int villageMovePriority = cv.directUnitMovePriority(village);

    Unit *castel = cv.createDummyUnit(102, 20, 20, 50000, CASTEL);
    int castelMovePriority = cv.directUnitMovePriority(castel);

    Unit *worker2 = cv.createDummyUnit(103, 50, 50, 2000, WORKER);
    int workerMovePirority2 = cv.directUnitMovePriority(worker2);

    Unit *assasin = cv.createDummyUnit(104, 40, 40, 5000, ASSASIN);
    assasin->role = LEADER;
    int leaderMovePriority = cv.directUnitMovePriority(assasin);

    Unit *assasin2 = cv.createDummyUnit(105, 40, 40, 5000, ASSASIN);
    assasin2->role = COMBATANT;
    int combatMovePriority = cv.directUnitMovePriority(assasin2);

    if(workerMovePirority > villageMovePriority) return false;
    if(villageMovePriority < castelMovePriority) return false;
    if(workerMovePirority >= workerMovePirority2) return false;
    if(leaderMovePriority < villageMovePriority) return false;
    if(leaderMovePriority < combatMovePriority) return false;

    return true;
  }

  /*
   * Case27: 敵ユニットの追加ができているかどうか
   */
  bool testCase27(){
    cv.stageInitialize();

    cv.addEnemyUnit(100, 10, 10, 2000, WORKER);
    if(enemyActiveUnitList.size() != 1) return false;

    cv.addEnemyUnit(101, 80, 80, 50000, CASTEL);
    if(enemyActiveUnitList.size() != 2) return false;
    if(enemyCastelCoordY != 80 && enemyCastelCoordX != 80) return false;

    return true;
  }

  /*
   * Case28: 試合状況が確認できているかどうか
   */
  bool testCase28(){
    cv.stageInitialize();

    cv.updateGameSituation();
    if(gameStage.gameSituation != OPENING) return false;

    cv.addEnemyUnit(100, 10, 10, 2000, WORKER);
    cv.updateGameSituation();
    if(gameStage.gameSituation != WARNING) return false;

    return true;
  }

  /*
   * Case29: ユニットの役割がちゃんと割り振れているかどうか
   */
  bool testCase29(){
    cv.stageInitialize(); 

    Unit *worker = cv.createDummyUnit(100, 10, 10, 2000, WORKER);

    if(worker->role != WORKER) return false;

    gameStage.field[10][10].myUnitCount[ASSASIN] = 1;
    Unit *assasin = cv.createDummyUnit(101, 10, 10, 5000, ASSASIN);
    if(assasin->role != LEADER) return false;

    gameStage.field[10][10].myUnitCount[ASSASIN] = 2;
    Unit *assasin2 = cv.createDummyUnit(102, 10, 10, 5000, ASSASIN);
    if(assasin2->role != COMBATANT) return false;
    if(assasin2->leaderId != 101) return false;

    return true;
  }

  /*
   * Case30: 目的地のリセットが出来ているかどうか
   */
  bool testCase30(){
    cv.stageInitialize();

    Unit *unit = cv.createDummyUnit(100, 10, 10, 5000, WORKER);
    cv.resetDestPoint(unit);

    if(unit->destY != UNDEFINED || unit->destX != UNDEFINED) return false;

    return true;
  }

  /*
   * Case31: 破壊モードに移行出来ているかどうか
   */
  bool testCase31(){
    cv.stageInitialize();
    gameStage.field[10][10].myUnitCount[ASSASIN] = 1;

    Unit *assasin = cv.createDummyUnit(100, 10, 10, 5000, ASSASIN);
    if(assasin->role != LEADER) return false;

    assasin->troopsCount = ATTACK_NUM;
    assasin->mode = cv.directUnitMode(assasin);

    if(assasin->mode != DESTROY) return false;

    return true;
  }

  /*
   * Case32: 自軍のユニットが生きているかの確認
   */
  bool testCase32(){
    cv.stageInitialize();

    myActiveUnitList.insert(100);

    if(!cv.isAlive(100)) return false;
    if(cv.isAlive(101)) return false;

    return true;
  }

  /*
   * Case33: 行動の評価値がちゃんとしている
   */
  bool testCase33(){
    cv.stageInitialize();

    Unit *assasin = cv.createDummyUnit(0, 10, 10, 5000, ASSASIN);
    if(assasin->role != LEADER) return false;

    Unit *assasin2 = cv.createDummyUnit(0, 10, 10, 5000, ASSASIN);
    if(assasin2->role != COMBATANT) return false;

    return true;
  }

  /*
   * Case34: ダミーユニットの作成が出来ているかどうか
   */
  bool testCase34(){
    cv.stageInitialize();

    Unit *assasin = cv.createDummyUnit(0, 10, 10, 5000, ASSASIN);
    if(gameStage.field[10][10].myUnitCount[ASSASIN] != 1) return false;

    Unit *assasin2 = cv.createDummyUnit(1, 10, 10, 5000, ASSASIN);
    if(gameStage.field[10][10].myUnitCount[ASSASIN] != 2) return false;
    Unit *assasin3 = cv.createDummyUnit(2, 11, 10, 5000, ASSASIN);
    Unit *assasin4 = cv.createDummyUnit(3, 10, 11, 5000, ASSASIN);

    if(assasin->mode != STAY) return false;
    if(assasin2->mode != STAY) return false;
    if(cv.calcManhattanDist(assasin2->y, assasin2->x, assasin->y, assasin->x) != 0) return false;

    assasin->mode = STAY;
    assasin2->mode = STAY;
    assasin3->mode = STAY;
    assasin4->mode = STAY;

    assasin3->role = COMBATANT;
    assasin3->leaderId = 0;

    assasin4->role = COMBATANT;
    assasin4->leaderId = 0;

    if(assasin->role != LEADER) return false;
    if(assasin2->role != COMBATANT) return false;
    if(assasin3->role != COMBATANT) return false;
    if(assasin4->role != COMBATANT) return false;

    if(cv.calcManhattanDist(assasin2->y, assasin2->x, assasin->y, assasin->x) != 0) return false;
    if(cv.calcManhattanDist(assasin3->y, assasin3->x, assasin->y, assasin->x) != 1) return false;

    assasin->troopsCount = 11;
    gameStage.field[10][10].myUnitCount[ASSASIN] = 11;

    gameStage.field[11][10].myUnitCount[ASSASIN] = 5;

    return true;
  }

  /*
   * リーダがDESTROYになったときに戦闘員もDESTROYになる
   */
  bool testCase35(){
    cv.stageInitialize();

    Unit *leader = cv.createDummyUnit(0, 10, 10, 5000, ASSASIN);
    Unit *combat = cv.createDummyUnit(1, 10, 10, 5000, ASSASIN);

    if(leader->role != LEADER) return false;
    if(combat->role != COMBATANT) return false;

    leader->mode = DESTROY;
    combat->mode = cv.directUnitMode(combat);

    if(combat->mode != DESTROY) return false;

    return true;
  }

  /*
   * 複数リーダが居るときには一番近いリーダに
   */
  bool testCase36(){
    cv.stageInitialize();

    Unit *leader1 = cv.createDummyUnit(0, 10, 10, 5000, ASSASIN);
    Unit *leader2 = cv.createDummyUnit(1, 80, 80, 5000, ASSASIN);

    Unit *combat = cv.createDummyUnit(2, 80, 80, 5000, ASSASIN);

    if(combat->leaderId != 1) return false;

    return true;
  }

  /*
   * Case37: 城からの攻撃を受けたかどうかの判定
   */
  bool testCase37(){
    cv.stageInitialize();

    Unit *assasin = cv.createDummyUnit(0, 10, 10, 5000, ASSASIN);
    if(cv.isCastelDamage(assasin)) return false;

    assasin->hp = 4900;
    cv.addEnemyUnit(1, 11, 11, 5000, WORKER);

    if(cv.isCastelDamage(assasin)) return false;

    assasin->hp = 4950;
    cv.createDummyUnit(2, 10, 10, 5000, ASSASIN);

    if(cv.isCastelDamage(assasin)) return false;

    assasin->hp = 4990;
    for(int i = 3; i < 11; i++){
      cv.createDummyUnit(i, 10, 10, 5000, ASSASIN);
    }
    if(cv.isCastelDamage(assasin)) return false;

    cv.createDummyUnit(11, 10, 10, 5000, ASSASIN);
    if(cv.isCastelDamage(assasin)) return false;

    assasin->y = 85;
    assasin->x = 85;

    cv.addEnemyUnit(12, 90, 90, 50000, CASTEL);
    if(!cv.isCastelDamage(assasin)) return false;

    // reset
    cv.stageInitialize();
    fprintf(stderr,"hello\n");
    Unit *worker = cv.createDummyUnit(0, 63, 90, 1900, WORKER);
    cv.addEnemyUnit(1, 67, 96, 50000, CASTEL);
    if(!cv.isCastelDamage(worker)) return false;

    // reset
    cv.stageInitialize();
    Unit *worker1 = cv.createDummyUnit(0, 94, 17, 1550, WORKER);
    worker1->beforeHp = 1566;

    Unit *village = cv.createDummyUnit(1, 94, 17, 19650, VILLAGE);
    village->beforeHp = 19666;

    Unit *worker2 = cv.createDummyUnit(2, 94, 17, 1700, WORKER);
    worker2->beforeHp = 1716;

    Unit *worker3 = cv.createDummyUnit(3, 94, 17, 1766, WORKER);
    worker3->beforeHp = 1782;

    Unit *worker4 = cv.createDummyUnit(4, 94, 17, 1816, WORKER);
    worker4->beforeHp = 1832;

    Unit *worker5 = cv.createDummyUnit(5, 94, 17, 1856, WORKER);
    worker5->beforeHp = 1872;

    cv.addEnemyUnit(6, 94, 17, 20000, VILLAGE);
    if(enemyActiveUnitList.size() != 1) return false;
    if(cv.isCastelDamage(worker1)) return false;

    return true;
  }

  /*
   * Case38: 自軍の数を数えることができる
   */
  bool testCase38(){
    cv.stageInitialize();

    cv.createDummyUnit(0, 10, 10, 5000, ASSASIN);
    cv.createDummyUnit(1, 10, 10, 5000, ASSASIN);

    int myUnitCount = cv.countMyUnit(10, 10, 2);

    if(myUnitCount != 2) return false;

    for(int i = 2; i < 11; i++){
      cv.createDummyUnit(i, 10, 10, 5000, ASSASIN);
    }

    myUnitCount = cv.countMyUnit(10, 10, 2);
    if(myUnitCount != 10) return false;

    cv.createDummyUnit(20, 63, 90, 2000, WORKER);
    myUnitCount = cv.countMyUnit(67, 96, 10);
    if(myUnitCount != 1) return false;

    return true;
  }

  /*
   * Case39: 敵の城の候補地を出す
   */
  bool testCase39(){
    cv.stageInitialize();

    cv.setCastelPointList(20, 20);

    if(gameStage.enemyCastelPointList.size() != 0) return false;

    return true;
  }

  /*
   * Case40: 近くの壁との距離が測れる
   */
  bool testCase40(){
    cv.stageInitialize();

    if(cv.calcNearWallDistance(0,0) != 0) return false;
    if(cv.calcNearWallDistance(3,2) != 2) return false;
    if(cv.calcNearWallDistance(10,20) != 10) return false;
    if(cv.calcNearWallDistance(99,98) != 0) return false;

    return true;
  }

  /*
   * Case41: 敵の生存確認ができる
   */
  bool testCase41(){
    cv.stageInitialize();
    cv.addEnemyUnit(0, 10, 10, 2000, WORKER);
    Unit *enemy = &unitList[0];
    Node *node = &gameStage.field[10][10];

    enemy->timestamp = -1;
    node->timestamp = turn;
    if(enemyActiveUnitList.size() != 1) return false;

    cv.enemyUnitSurvivalCheck();

    if(enemyActiveUnitList.size() != 0) return false;

    return true;
  }

  /*
   * Case42: 収入のカウントが出来ている
   */
  bool testCase42(){
    cv.stageInitialize();

    cv.addResourceNode(10, 10);
    cv.updateIncomeResource();
    if(gameStage.incomeResource != 10) return 0;

    gameStage.field[10][10].myUnitCount[WORKER] = 1;
    cv.updateIncomeResource();
    if(gameStage.incomeResource != 11) return 0;

    gameStage.field[10][10].myUnitCount[WORKER] = 6;
    cv.updateIncomeResource();
    if(gameStage.incomeResource != 15) return 0;

    return true;
  }

  /*
   * Case43: 敵の城である可能性があるかどうかを確かめる
   */
  bool testCase43(){
    cv.stageInitialize();

    cv.addEnemyUnit(0, 90, 72, 50000, CASTEL);
    gameStage.field[95][67].searched = true;

    if(!cv.isCastelPoint(90, 72)) return false;

    return true;
  }

  /*
   * Case44: 敵の攻撃を受ける範囲をマークつけることが出来る
   */
  bool testCase44(){
    cv.stageInitialize();

    Unit *enemy = cv.createDummyEnemyUnit(0, 50, 50, 2000, WORKER);
    cv.checkEnemyMark(enemy);

    Node *node = cv.getNode(50, 50);

    if(node->enemyAttackCount[WORKER] != 1) return false;

    node = cv.getNode(50, 53);
    if(node->enemyAttackCount[WORKER] != 1) return false;

    node = cv.getNode(50, 54);
    if(node->enemyAttackCount[WORKER] != 0) return false;

    return true;
  }

  /*
   * Case45: 敵の攻撃のダメージの計算を行う
   */
  bool testCase45(){
    cv.stageInitialize();

    Unit *enemy   = cv.createDummyEnemyUnit(0, 50, 50, 5000, KNIGHT);
    Unit *worker  = cv.createDummyUnit(1, 50, 49, 2000, WORKER);
    Unit *knight  = cv.createDummyUnit(2, 50, 48, 5000, KNIGHT);
    Unit *fighter = cv.createDummyUnit(3, 50, 47, 5000, FIGHTER);
    Unit *assasin = cv.createDummyUnit(4, 50, 46, 5000, ASSASIN);

    cv.checkEnemyMark(enemy);

    if(cv.calcReceivedCombatDamage(worker) != 100) return false;
    if(cv.calcReceivedCombatDamage(knight) != 500) return false;
    if(cv.calcReceivedCombatDamage(fighter) != 200) return false;
    if(cv.calcReceivedCombatDamage(assasin) != 0) return false;

    return true;
  }

  /*
   * Case46: 資源マスが占領されているかどうかを調べる
   */
  bool testCase46(){
    cv.stageInitialize();

    cv.addResourceNode(50, 50);
    if(cv.isOccupied(50, 50)) return false;

    for(int i = 0; i < 5; i++){
      cv.addEnemyUnit(i, 50, 50, 2000, WORKER);
    }

    if(!cv.isOccupied(50, 50)) return false;

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
