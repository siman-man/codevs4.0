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
const int WORKER    = 0; // ワーカー
const int KNIGHT    = 1; // ナイト
const int FIGHER    = 2; // ファイター
const int ASSASIN   = 3; // アサシン
const int CASTEL    = 4; // 城
const int VILLAGE   = 5; // 村
const int BASE      = 6; // 拠点
const int COMBATANT = 7; // 戦闘員
const int LEADER    = 8; // 戦闘隊長
const int COLLIERY  = 9; // 炭鉱(資源マスにワーカーが5人いる状態)

// 行動の基本優先順位
const int movePriority[10] = { 5, 9, 8, 7, 0, 10, 15, 17, 20, 1};

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
const int COST_MAX = 99999;     // コストの最大値(城を事実上作れなくする)
const int MIN_VALUE = -999999;  // 最小値

// 座標計算で使用する配列
const int dy[5] = {0,-1, 1, 0, 0};
const int dx[5] = {0, 0, 0,-1, 1};

// その他
const int UNKNOWN = -1;       // 未知
const int UNDEFINED = -1;     // 未定
const int VIRTUAL_ID = 30000; // 仮想ID 
const int REAL = true;        // 確定コマンド

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

// 行動の優先順位
struct MovePriority{
  int unitId;   // ユニットID
  int priority; // 優先度

  bool operator >(const MovePriority &e) const{
    return priority < e.priority;
  }    
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
  int resourceY;          // 目的地(資源)のy座標
  int resourceX;          // 目的地(資源)のx座標
  int createWorkerCount;  // 生産したワーカーの数
  int hp;                 // HP
  int type;               // ユニットの種別
  int eyeRange;           // 視野
  int attackRange;        // 攻撃範囲
  bool  movable;          // 移動できるかどうか
  int timestamp;          // 更新ターン
};

// フィールドの1マスに対応する
struct Node{
  bool resource;            // 資源マスかどうか
  bool opened;              // 調査予定マス
  bool searched;            // 既に調査済みかどうか
  bool rockon;              // ノードを狙っている自軍がいるかどうか
  int stamp;                // 足跡
  int cost;               // ノードのコスト
  int markCount;          // マークカウント(自軍が行動する予定のマス)
  int seenCount;          // ノードを監視しているユニットの数 
  int myUnitCount[7];     // 自軍の各ユニット数
  int enemyUnitCount[7];  // 相手の各ユニット数
  set<int> seenMembers;   // ノードを監視している自軍のメンバー
};

// ゲーム・フィールド全体の構造
struct GameStage{
  int searchedNodeCount;      // 調査済みのマスの数
  int openedNodeCount;        // 調査予定マスの数
  int visibleNodeCount;       // 現在確保できている視界の数   
  int gameSituation;          // 試合状況
  Node field[HEIGHT][WIDTH];  // ゲームフィールド
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
      fprintf(stderr,"stageInitialize =>\n");
      // ユニットのチェックリストの初期化
      unitIdCheckList.clear();

      // アクティブユニットリストの初期化
      myActiveUnitList.clear();

      // 敵ユニットリストの初期化
      enemyActiveUnitList.clear();

      // 資源マスの初期化
      resourceNodeList.clear();

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

      // 自軍の城の座標をリセット
      myCastelCoordY = UNKNOWN;
      myCastelCoordX = UNKNOWN;

      // 敵軍の城の座標をリセット
      enemyCastelCoordY = UNKNOWN;
      enemyCastelCoordX = UNKNOWN;
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

      // 資源数
      scanf("%d", &myResourceCount);

      // 自軍のユニット数
      scanf("%d", &myAllUnitCount);

      // 自軍ユニットの詳細
      for(int i = 0; i < myAllUnitCount; i++){
        scanf("%d %d %d %d %d", &unitId, &y, &x, &hp, &unitType);


        gameStage.field[y][x].myUnitCount[unitType] += 1;

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
      unit.type         = unitType;
      unit.destY        = UNDEFINED;
      unit.destX        = UNDEFINED;
      unit.createWorkerCount = 0;
      unit.attackRange  = unitAttackRange[unitType];
      unit.eyeRange     = unitEyeRange[unitType];
      unit.movable      = unitCanMove[unitType];
      unit.timestamp    = turn;

      // 自軍の城の座標を更新
      if(unitType == CASTEL){
        myCastelCoordY = y;
        myCastelCoordX = x;
      }

      unitList[unitId] = unit;
      unitList[unitId].mode = directFirstMode(&unitList[unitId]);
      unitList[unitId].role = directUnitRole(&unitList[unitId]);
      myActiveUnitList.insert(unitId);
      unitIdCheckList[unitId] = true;
      checkNode(unitId, y, x, unit.eyeRange);

      if(unit.mode == SEARCH){
        //checkStamp(y, x, unit.eyeRange * 2);
      }
      if(unit.role == COMBATANT){
        set<int>::iterator it = myActiveUnitList.begin();

        while(it != myActiveUnitList.end()){
          Unit *other = &unitList[*it];

          if(calcManhattanDist(unit.y, unit.x, other->y, other->x) == 0 && other->mode == LEADER){
            unitList[unitId].destY = other->y;
            unitList[unitId].destX = other->x;
          }

          it++;
        }
      }
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

      // 敵軍の城の座標を更新
      if(unitType == CASTEL){
        enemyCastelCoordY = y;
        enemyCastelCoordX = x;
      }

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
     * 最初のモードを決める
     */
    int directFirstMode(Unit *unit){
      Node *node = &gameStage.field[unit->y][unit->x];

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
        case ASSASIN:
          if(node->myUnitCount[ASSASIN] == 1){
            return SEARCH;
          }else{
            return SEARCH;
          }
          break;
        default:
          break;
      }

      return NONE;
    }

    /*
     * 次の目的地を決める
     */
    Coord directNextPoint(Unit *unit){
      Coord bestCoord;

      if(turn >= 250) return Coord(80, 80);

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

      return Coord(99, 99);
    }

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
      node.seenCount = 0;
      node.cost = 0;
      node.stamp = 0;
      node.markCount = 0;
      node.resource = false;
      node.opened = false;
      node.rockon = false;
      node.searched = false;

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
      openNode(VIRTUAL_ID, y, x, unitEyeRange[unitType]);
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
      unit->hp        = hp;
      unit->timestamp = turn;

      //checkNode(unitId, y, x, unit->eyeRange);
      if(unit->mode == SEARCH){
        //checkStamp(y, x, unit->eyeRange * 2);
      }
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
    }

    /*
     * ユニットのモードの状態の更新を行う
     */
    void updateUnitMode(){
      set<int>::iterator it = myActiveUnitList.begin();

      while(it != myActiveUnitList.end()){
        Unit *unit = &unitList[*it];
        unit->mode = directUnitMode(unit);

        // SEARCHモードのユニットの目的地の設定されていない場合、更新する。
        if(unit->mode == SEARCH && (gameStage.field[unit->destY][unit->destX].searched || (unit->destY == UNDEFINED && unit->destX == UNDEFINED))){
          Coord coord = directNextPoint(unit);

          if(enemyCastelCoordY != UNDEFINED){
            unit->destY = enemyCastelCoordY;
            unit->destX = enemyCastelCoordX;
          }else{
            unit->destY = coord.y;
            unit->destX = coord.x;
          }

          checkMark(unit->destY, unit->destX);
        }

        it++;
      }
    }

    /*
     * ユニットの状態を決定する
     */
    int directUnitMode(Unit *unit){
      switch(unit->type){
        case WORKER:
          if(unit->mode == PICKING || pickModeCheck(unit)){
            return PICKING;
          }else{
            return SEARCH;
          }
          break;
        case KNIGHT:
          break;
        case FIGHER:
          break;
        case ASSASIN:
          if(unit->mode == LEADER){
            return SEARCH;
            return LEADER;
          }else{
            return SEARCH;
            return COMBATANT;
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
      if(gameStage.gameSituation != WARNING && enemyActiveUnitList.size() == 0){
        gameStage.gameSituation = OPENING;
      }else if(enemyCastelCoordY != UNDEFINED && enemyCastelCoordX != UNDEFINED){
        gameStage.gameSituation = ONRUSH;
      }else{
        gameStage.gameSituation = WARNING;
      }
    }

    /*
     * 役割を決める
     */
    int directUnitRole(Unit *unit){
      if(unit->type == ASSASIN){
        if(gameStage.field[unit->y][unit->x].myUnitCount[unit->type] == 1){
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
      return 1000 * movePriority[unit->type] - calcManhattanDist(unit->y, unit->x, 79, 79);
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

        if(!gameStage.field[y][x].rockon && checkMinDist(y, x, dist)){
          gameStage.field[y][x].rockon = true;
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
        if(minDist > dist && unit->mode != PICKING) return false;

        it++;
      }

      return true;
    }

    /*
     * 自軍の生存確認
     * ユニットのtimestampが更新されていない場合は前のターンで的に倒されたので、
     * リストから排除する。
     */
    void unitSurvivalCheck(){
      set<int> tempList = myActiveUnitList;
      set<int>::iterator it = tempList.begin();

      while(it != tempList.end()){
        Unit *unit = &unitList[*it];

        if(unit->timestamp != turn){
          removeMyUnit(unit);
        }

        it++;
      }
    }

    /*
     * 評価値の計算
     */
    int calcEvaluation(Unit *unit, int operation){
      int centerDist = calcManhattanDist(unit->y, unit->x, 0, 0);
      int destDist = (unit->mode == SEARCH)? calcManhattanDist(unit->y, unit->x, unit->destY, unit->destX) : 0;
      int stamp = gameStage.field[unit->y][unit->x].stamp;

      switch(unit->type){
        case WORKER:
          switch(unit->mode){
            case SEARCH:
              if(operation == NO_MOVE){
                return -10000;
              }else{
                //return 100 * myResourceCount + calcUnknownPoint(unit->y, unit->x);
                return 100 * myResourceCount + gameStage.openedNodeCount * 5 - destDist;
              }
              break;
            case PICKING:
              return calcPikingEvaluation(unit, operation);
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
    int calcPikingEvaluation(Unit *unit, int operation){
      Node *node = &gameStage.field[unit->y][unit->x];

      if(node->resource){
        //fprintf(stderr,"Base Count = %d\n", node->myUnitCount[BASE]);
        if(gameStage.gameSituation == WARNING && operation == CREATE_BASE && node->myUnitCount[BASE] == 1){
          return 10000;
        }else if(operation == CREATE_VILLAGE && node->myUnitCount[VILLAGE] == 1){
          return 100;
        }else{
          return 10 * node->myUnitCount[WORKER];
        }
      }else{
        return -calcManhattanDist(unit->y, unit->x, unit->resourceY, unit->resourceX);
      }
    }

    /*
     * NONE
     */
    int calcNoneEvaluation(Unit *unit, int operation){
      Node *node = &gameStage.field[unit->y][unit->x];

      if(operation == CREATE_WORKER && node->myUnitCount[WORKER] <= 5){
        return 100;
      }else{
        return 0;
      }
    }

    /*
     * 村が動いていない時の評価値
     */
    int calcNoneVillageEvaluation(Unit *unit, int operation){
      int castelDist = calcManhattanDist(unit->y, unit->x, 50, 50);
      Node *node = &gameStage.field[unit->y][unit->x];

      if(operation == CREATE_WORKER && node->myUnitCount[WORKER] <= 6 && unit->createWorkerCount <= 5){
        return 100;
      }else if(operation != CREATE_WORKER && gameStage.gameSituation == WARNING){
        return 1000;
      }else if(operation == CREATE_WORKER && myResourceCount >= 100 && castelDist <= 50 && unit->createWorkerCount <= 10){
        return 100;
      }else{
        return 0;
      }
    }

    /*
     * 城が動いていない時の評価値
     */
    int calcNoneCastelEvaluation(Unit *unit, int operation){
      if(operation == CREATE_WORKER && turn <= 16){
        return 100;
      }else{
        return 0;
      }
    }

    /*
     * 拠点が動いていない時の評価値
     */
    int calcNoneBaseEvaluation(Unit *unit, int operation){
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
      if(unit->mode == ONRUSH){
        return -1 * calcManhattanDist(unit->y, unit->x, enemyCastelCoordY, enemyCastelCoordX);
      }else if(gameStage.field[unit->y][unit->x].myUnitCount[unit->type] <= 4){
        return -1 * calcManhattanDist(unit->y, unit->x, unit->destY, unit->destX);
        if(operation == NO_MOVE){
          return 100;
        }else{
          return 0;
        }
      }else{
        return -1 * calcManhattanDist(unit->y, unit->x, 99, 99);
      }
    }

    /*
     *
     */
    int calcCombatEvaluation(Unit *unit, int operation){

      return 0;
    }

    /*
     * 自軍ユニットとの距離
     */
    int aroundMyUnitDist(Unit *unit){
      int dist;
      int sumDist = 0;
      priority_queue< Coord, vector<Coord>, greater<Coord>  > que;

      set<int>::iterator it = myActiveUnitList.begin();

      while(it != myActiveUnitList.end()){
        Unit *other = &unitList[*it];

        if(other->movable){
          Coord coord(other->y, other->x);
          dist = calcManhattanDist(unit->y, unit->x, other->y, other->x);
          coord.dist = dist;
        }

        it++;
      }

      for(int i = 0; i < 2 && !que.empty(); i++){
        sumDist += que.top().dist; que.pop();
      }

      return sumDist;
    }

    /*
     * コストを取得する
     */
    int checkCost(int ypos, int xpos, int dist = 3){
      map<int, bool> checkList;
      queue<cell> que;
      que.push(cell(Coord(ypos, xpos), dist));
      int cost = 0;

      while(!que.empty()){
        cell c = que.front(); que.pop(); 
        Coord coord = c.first;
        dist = c.second;

        if(checkList[coord.y*WIDTH+coord.x] || dist < 0) continue;
        checkList[coord.y*WIDTH+coord.x] = true;

        if(gameStage.field[coord.y][coord.x].searched || gameStage.field[coord.y][coord.x].markCount > 0){
          cost -= 3;
        }else{
          cost += 8;
        }


        for(int i = 1; i < 5; i++){
          int ny = coord.y + dy[i];
          int nx = coord.x + dx[i];
          if(!isWall(ny,nx)) que.push(cell(Coord(ny, nx), dist-1));
        }
      }

      return cost;
    }

    /*
     * マークを付ける
     */
    void checkMark(int ypos, int xpos){
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
          node->cost = 0;
          if(turn % 10 == 0){
            node->markCount = 0;
          }
          node->seenMembers.clear();
          node->opened = false;
          node->stamp = 0;

          memset(node->myUnitCount, 0, sizeof(node->myUnitCount));
          memset(node->enemyUnitCount, 0, sizeof(node->enemyUnitCount));
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
        fprintf(stderr, "Remaing time is %dms\n", remainingTime);

        // フィールドのクリア
        clearField();

        // 各ターンで行う処理(主に入力の処理)
        eachTurnProc();

        // 自軍の生存確認
        unitSurvivalCheck();

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
        fprintf(stderr,"finalOperation: size = %d\n", size);
        fprintf(stderr,"openedNodeCount = %d\n", gameStage.openedNodeCount);
        fprintf(stderr,"visibleNodeCount = %d\n", gameStage.visibleNodeCount);
      }

      printf("%d\n", size);
      for(int i = 0; i < size; i++){
        Operation ope = operationList[i];
        printf("%d %c\n", ope.unitId, instruction[ope.operation]);
      }
    }

    /*
     * 視界をチェックする
     */
    void checkNode(int unitId, int ypos, int xpos, int eyeRange){
      for(int y = max(0, ypos-eyeRange); y <= min(HEIGHT-1, ypos+eyeRange); y++){
        int diff = 2*abs(ypos-y)/2;

        for(int x = max(0, xpos-eyeRange+diff); x <= min(WIDTH-1, xpos+eyeRange-diff); x++){
          if(isWall(y,x)) continue;

          gameStage.field[y][x].seenMembers.insert(unitId);
          gameStage.field[y][x].seenCount += 1;
          if(unitList[unitId].type == WORKER){
            gameStage.field[y][x].cost -= 1;
          }

          if(!gameStage.field[y][x].searched){
            //fprintf(stderr,"searchedNodeCount = %d\n", gameStage.searchedNodeCount);
            gameStage.searchedNodeCount += 1;
            gameStage.field[y][x].searched = true;
          }

          if(!gameStage.field[y][x].opened){
            gameStage.visibleNodeCount += 1;
            gameStage.field[y][x].opened = true;
          }
        }
      }
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
     * 視界をオープンする
     */
    void openNode(int unitId, int ypos, int xpos, int eyeRange){
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
     * 視界をクローズする
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
     */
    bool unitAction(Unit *unit, int type, bool final = false){
      switch(type){
        case MOVE_UP:
          if(canMove(unit->y, unit->x, MOVE_UP)){
            closeNode(unit->y, unit->x, unit->eyeRange);
            moveUp(unit);

            if(final){
              checkNode(unit->id, unit->y, unit->x, unit->eyeRange);
            }else{
              openNode(unit->id, unit->y, unit->x, unit->eyeRange);
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
              openNode(unit->id, unit->y, unit->x, unit->eyeRange);
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
              openNode(unit->id, unit->y, unit->x, unit->eyeRange);
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
              openNode(unit->id, unit->y, unit->x, unit->eyeRange);
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
          }else{
            return false;
          }
          break;
        case CREATE_FIGHER:
          if(canBuild(unit->type, FIGHER)){
            createUnit(unit->y, unit->x, FIGHER);
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
          openNode(unit->id, unit->y, unit->x, unit->eyeRange);
          break;
        case MOVE_DOWN:
          closeNode(unit->y, unit->x, unit->eyeRange);
          moveUp(unit);
          openNode(unit->id, unit->y, unit->x, unit->eyeRange);
          break;
        case MOVE_LEFT:
          closeNode(unit->y, unit->x, unit->eyeRange);
          moveRight(unit);
          openNode(unit->id, unit->y, unit->x, unit->eyeRange);
          break;
        case MOVE_RIGHT:
          closeNode(unit->y, unit->x, unit->eyeRange);
          moveLeft(unit);
          openNode(unit->id, unit->y, unit->x, unit->eyeRange);
          break;
        case CREATE_WORKER:
          deleteUnit(unit->y, unit->x, WORKER);
          break;
        case CREATE_KNIGHT:
          deleteUnit(unit->y, unit->x, KNIGHT);
          break;
        case CREATE_FIGHER:
          deleteUnit(unit->y, unit->x, FIGHER);
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
     * 何も行動しない
     */
    void noMove(){
    }

    /*
     * 上に動く
     */
    void moveUp(Unit *unit){
      unit->y -= 1;
    }

    /*
     * 下に動く
     */
    void moveDown(Unit *unit){
      unit->y += 1;
    }

    /*
     * 左に動く
     */
    void moveLeft(Unit *unit){
      unit->x -= 1;
    }

    /*
     * 右に動く
     */
    void moveRight(Unit *unit){
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
        Unit *unit = &unitList[*it];
        MovePriority mp;
        mp.unitId = (*it);
        mp.priority = directUnitMovePriority(unit);
        prique.push(mp);
        it++;
      }


      while(!prique.empty()){
        MovePriority mp = prique.top(); prique.pop();
        Unit *unit = &unitList[mp.unitId];

        if(unit->type == WORKER && gameStage.gameSituation == ONRUSH) continue;

        priority_queue<Operation, vector<Operation>, greater<Operation> > que;
        gameStage.searchedNodeCount = 0;

        tempGameStage = gameStage;

        fprintf(stderr, "turn = %d, unitId = %d, y = %d, x = %d, mode = %d, type = %d\n", turn, unit->id, unit->y, unit->x, unit->mode, unit->type);

        for(int operation = 0; operation < OPERATION_MAX; operation++){

          if(!OPERATION_LIST[unit->type][operation]) continue;
          //fprintf(stderr,"operation = %d\n", operation);
          int onc = gameStage.openedNodeCount;
          int snc = gameStage.searchedNodeCount;
          int vnc = gameStage.visibleNodeCount;

          // 行動が成功した時だけ評価を行う
          if(unitAction(unit, operation)){

            Operation ope;
            ope.unitId = unit->id;
            ope.operation = operation;
            ope.evaluation = calcEvaluation(unit, operation);

            /*
               fprintf(stderr,"y = %d, x = %d\n", unit->y, unit->x);
               fprintf(stderr,"vnc = %d, gameStage.visibleNodeCount = %d\n", vnc, gameStage.visibleNodeCount);
               */
            // 行動を元に戻す
            rollbackAction(unit, operation);

            que.push(ope);
          }else{
            //fprintf(stderr,"Failed operation = %d\n", operation);
          }

          gameStage = tempGameStage;
          // 元に戻っていない場合はエラー
          //fprintf(stderr,"vnc = %d, gameStage.visibleNodeCount = %d\n", vnc, gameStage.visibleNodeCount);
          assert(snc == gameStage.searchedNodeCount);
          assert(vnc == gameStage.visibleNodeCount);
          assert(onc == gameStage.openedNodeCount);
        }

        Operation bestOperation = que.top();

        // 行動なし以外はリストに入れる
        if(bestOperation.operation != NONE){
          operationList.push_back(bestOperation);

          // 確定した行動はそのままにする
          unitAction(unit, bestOperation.operation, REAL);
          fprintf(stderr,"unitId = %d, action = %d, value = %d, searchedCount = %d\n", unit->id, bestOperation.operation, bestOperation.evaluation, gameStage.searchedNodeCount);

          if(unit->mode == SEARCH){
            checkStamp(unit->y, unit->x, unit->eyeRange * 2);
          }

          // SEARCHモードのユニットが目的地に到達した場合は、目的地の座標をリセット
          if(unit->mode == SEARCH && (unit->y == unit->destY && unit->x == unit->destX)){
            uncheckMark(unit->destY, unit->destX);
            unit->destY = UNDEFINED;
            unit->destX = UNDEFINED;
          }
        }
      }

      return operationList;
    }

    /*
     * 渡された座標のマンハッタン距離を計算
     */
    int calcManhattanDist(int y1, int x1, int y2, int x2){
      return manhattanDist[x1*WIDTH+x2] + manhattanDist[y1*WIDTH+y2];
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
    if(gameStage.searchedNodeCount != 0) return false;
    if(gameStage.visibleNodeCount != 0) return false;
    if(gameStage.openedNodeCount != 0) return false;

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
    Unit *unit = &unitList[0];
    int x = unit->x;
    int y = unit->y;

    cv.moveUp(unit);

    return (x == unit->x && y-1 == unit->y);
  }

  /*
   * Case7: 「下に移動」が出来ているかどうか
   */
  bool testCase7(){
    Unit *unit = &unitList[0];
    int x = unit->x;
    int y = unit->y;

    cv.moveDown(unit);

    return (x == unit->x && y+1 == unit->y);
  }

  /*
   * Case8: 「左に移動」が出来ているかどうか
   */
  bool testCase8(){
    Unit *unit = &unitList[0];
    int x = unit->x;
    int y = unit->y;

    cv.moveLeft(unit);

    return (x-1 == unit->x && y == unit->y);
  }

  /*
   * Case9: 「右に移動」が来ているかどうか
   */
  bool testCase9(){
    Unit *unit = &unitList[0];
    int x = unit->x;
    int y = unit->y;

    cv.moveRight(unit);

    return (x+1 == unit->x && y == unit->y);
  }

  /*
   * Case10: 「生産可否判定」が出来ているかどうか
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
    if(cv.canBuild(village->type, KNIGHT)) return false;

    myResourceCount = 40;
    if(!cv.canBuild(village->type, WORKER)) return false;
    if(!cv.canBuild(castel->type, WORKER)) return false;
    if(!cv.canBuild(base->type, KNIGHT)) return false;
    if(!cv.canBuild(base->type, FIGHER)) return false;
    if(cv.canBuild(base->type, ASSASIN)) return false;
    if(cv.canBuild(village->type, FIGHER)) return false;

    myResourceCount = 60;
    if(!cv.canBuild(base->type, ASSASIN)) return false;
    if(cv.canBuild(castel->type, ASSASIN)) return false;
    if(cv.canBuild(worker->type, VILLAGE)) return false;

    myResourceCount = 100;
    if(!cv.canBuild(worker->type, VILLAGE)) return false;
    if(!cv.canBuild(base->type, KNIGHT)) return false;
    if(!cv.canBuild(base->type, FIGHER)) return false;
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
    if(node.myUnitCount[FIGHER] != 0) return false;
    if(node.myUnitCount[BASE] != 0) return false;
    if(node.enemyUnitCount[WORKER] != 0) return false;
    if(node.enemyUnitCount[FIGHER] != 0) return false;
    if(node.enemyUnitCount[BASE] != 0) return false;
    if(node.seenMembers.size() != 0) return false;
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

    if(myActiveUnitList.size() != 3) return false;


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

    cv.unitSurvivalCheck();

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
    if(cv.unitAction(unit, CREATE_FIGHER)) return false;
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

    int unitId = 100;
    myResourceCount = COST_MAX;
    cv.addMyUnit(unitId, 10, 10, 2000, WORKER);
    unitId = 101;
    cv.addMyUnit(unitId, 11, 11, 2000, WORKER);

    Unit *unitA = &unitList[100];
    Unit *unitB = &unitList[101];
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
    int unitId = 100;

    cv.addResourceNode(10, 10);
    cv.addMyUnit(unitId, 10, 10, 2000, WORKER);

    Unit *unit = &unitList[unitId];

    if(unit->resourceY != 10 || unit->resourceX != 10) return false;
    if(unit->mode != PICKING) return false;

    gameStage.field[10][10].myUnitCount[WORKER] = 6;
    unitId = 101;
    cv.addMyUnit(unitId, 10, 10, 2000, WORKER);
    unit = &unitList[unitId];
    if(unit->mode == PICKING) return false;

    return true;
  }

  /*
   * Case26: 行動の優先順位が設定されているかどうか
   */
  bool testCase26(){
    cv.stageInitialize(); 

    int unitId = 100;
    cv.addMyUnit(unitId, 10, 10, 2000, WORKER);
    Unit *worker = &unitList[unitId];
    int workerMovePirority = cv.directUnitMovePriority(worker);

    unitId = 101;
    cv.addMyUnit(unitId, 11, 11, 2000, VILLAGE);
    Unit *village = &unitList[unitId];
    int villageMovePriority = cv.directUnitMovePriority(village);

    unitId = 102;
    cv.addMyUnit(unitId, 20, 20, 50000, CASTEL);
    Unit *castel = &unitList[unitId];
    int castelMovePriority = cv.directUnitMovePriority(castel);

    unitId = 103;
    cv.addMyUnit(unitId, 50, 50, 2000, WORKER);
    Unit *worker2 = &unitList[unitId];
    int workerMovePirority2 = cv.directUnitMovePriority(worker2);

    if(workerMovePirority > villageMovePriority) return false;
    if(villageMovePriority < castelMovePriority) return false;
    if(workerMovePirority >= workerMovePirority2) return false;

    return true;
  }

  /*
   * Case27: 敵ユニットの追加ができているかどうか
   */
  bool testCase27(){
    cv.stageInitialize();

    int unitId = 100;

    cv.addEnemyUnit(unitId, 10, 10, 2000, WORKER);
    if(enemyActiveUnitList.size() != 1) return false;

    unitId = 101;
    cv.addEnemyUnit(unitId, 80, 80, 50000, CASTEL);
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

    int unitId = 100;
    cv.addEnemyUnit(unitId, 10, 10, 2000, WORKER);
    cv.updateGameSituation();
    if(gameStage.gameSituation != WARNING) return false;

    return true;
  }

  /*
   * Case29: ユニットの役割がちゃんと割り振れているかどうか
   */
  bool testCase29(){
    cv.stageInitialize(); 

    int unitId = 100;

    cv.addMyUnit(unitId, 10, 10, 2000, WORKER);
    Unit *worker = &unitList[unitId];

    if(worker->role != WORKER) return false;

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
