#include <iostream>
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

const int MAX_UNIT_ID  = 20010; // ユニットのIDの上限
const int HEIGHT = 100; // フィールドの横幅
const int WIDTH = 100;  // フィールドの縦幅

// プレイヤーの名前
const string PLAYER_NAME = "siman";

// ダメージテーブル
const int DAMAGE_TABLE[7][7] = {
  { 100, 100, 100, 100, 100, 100, 100},
  { 100, 500, 200, 200, 200, 200, 200},
  { 500,1600, 500, 200, 200, 200, 200},
  {1000, 500,1000, 500, 200, 200, 200},
  { 100, 100, 100, 100, 100, 100, 100},
  { 100, 100, 100, 100, 100, 100, 100},
  { 100, 100, 100, 100, 100, 100, 100}
};

// ユニットが持つ構造
struct Unit{
  int id;           // ユニットの種別を表すID
  int hp;           // HP
  int eyeRange;     // 視野
  int attackRange;  // 攻撃範囲
  bool canMove;     // 移動できるかどうか
};

int remainingTime;          // 残り時間
int stageNumber = 0;       // 現在のステージ数
int turn;                   // 現在のターン
int myUnitCount;            // 自軍のユニット数
int enemyUnitCount;         // 敵軍のユニット数
int resourceCount;          // 資源の数
Unit* unitList[MAX_UNIT_ID]; // ユニットのリスト

char field[HEIGHT][WIDTH];  // ゲームフィールド


class Codevs{
  public:
    /*
     * ゲームの初期化
     */
    void init(){
      memset(field, -1, sizeof(field));
    }

    /*
     * 各ターンの入力処理
     */
    void eachTurnProc(){
      // 残り時間(ms)
      scanf("%d", &remainingTime);

      // 現在のステージ数(0-index)
      scanf("%d", &stageNumber);
    }

    void run(){
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
};

int main(){
  return 0;
}
