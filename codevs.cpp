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

int currentStage = 0; // 現在のステージ数
int turn;             // 現在のターン

// ユニットが持つ構造
struct Unit{
  int id;           // ユニットの種別を表すID
  int hp;           // HP
  int eyeRange;     // 視野
  int attackRange;  // 攻撃範囲
  bool canMove;     // 移動できるかどうか
};


class Codevs{
  public:
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
