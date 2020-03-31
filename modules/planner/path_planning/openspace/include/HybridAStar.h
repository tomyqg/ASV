/*
*******************************************************************************
* HybridAStar.h:
* Hybrid A star algorithm, to generate the coarse collision-free trajectory
* This header file can be read by C++ compilers
*
* by Hu.ZH, Bin Li(CrossOcean.ai)
*******************************************************************************
*/

#ifndef _HYBRIDASTAR_H_
#define _HYBRIDASTAR_H_

#include "CollisionChecking.h"
#include "common/math/Geometry/include/Reeds_Shepp.h"
#include "hybridstlastar.h"
#include "openspacedata.h"

namespace ASV::planning {

constexpr int DEBUG_LISTS = 0;
constexpr int DEBUG_LIST_LENGTHS_ONLY = 0;

class HybridState4DNode {
  enum MovementType {
    STRAIGHT_FORWARD = 0,
    LEFTTURN_FORWARD,
    RIGHTTURN_FORWARD,
    STRAIGHT_REVERSE,
    RIGHTTURN_REVERSE,
    LEFTTURN_REVERSE
  };

 public:
  HybridState4DNode()
      : x_(0.0), y_(0.0), theta_(0.0), type_(STRAIGHT_FORWARD) {}

  HybridState4DNode(float px, float py, float ptheta,
                    MovementType ptype = STRAIGHT_FORWARD)
      : x_(px), y_(py), theta_(ptheta), type_(ptype) {}

  float x() const noexcept { return x_; }
  float y() const noexcept { return y_; }
  float theta() const noexcept { return theta_; }
  MovementType type() const noexcept { return type_; }

  // Here's the heuristic function that estimates the distance from a Node
  // to the Goal.
  float GoalDistanceEstimate(HybridState4DNode &nodeGoal) {
    // return abs(x - nodeGoal.x) + abs(y - nodeGoal.y);

    return (x_ - nodeGoal.x()) * (x_ - nodeGoal.x()) +
           (y_ - nodeGoal.y()) * (y_ - nodeGoal.y());
    // return 0;
  }  // GoalDistanceEstimate

  bool IsGoal(HybridState4DNode &nodeGoal) {
    if (std::hypot(x_ - nodeGoal.x(), y_ - nodeGoal.y()) < 0.1) {
      return true;
    }

    return false;
  }  // IsGoal

  // This generates the successors to the given Node. It uses a helper function
  // called AddSuccessor to give the successors to the AStar class. The A*
  // specific initialisation is done for each node internally, so here you just
  // set the state information that is specific to the application
  bool GetSuccessors(
      HybridAStarSearch<HybridState4DNode, SearchConfig> *astarsearch,
      HybridState4DNode *parent_node, const SearchConfig &_SearchConfig) {
    for (int i = 0; i != 6; ++i) {
      for (int j = 0; j != 6; ++j) {
        std::cout << _SearchConfig.cost_map[i][j] << " ";
      }
      std::cout << "\n";
    }

    float parent_x = -1.0;
    float parent_y = -1.0;

    if (parent_node) {
      parent_x = parent_node->x();
      parent_y = parent_node->y();
    }

    float L = 1;
    float max_turn = 0.2;

    auto move_step = update_movement_step(L, max_turn, this->theta_);

    // push each possible move except allowing the search to go backwards

    int x_int = static_cast<int>(x_);
    int y_int = static_cast<int>(y_);

    if (GetMap(x_int - 1, y_int) < 9) {
      HybridState4DNode NewNode =
          HybridState4DNode(x_int - 1, y_int, 0, STRAIGHT_FORWARD);

      astarsearch->AddSuccessor(NewNode);
    }

    if (GetMap(x_int, y_int - 1) < 9) {
      HybridState4DNode NewNode =
          HybridState4DNode(x_int, y_int - 1, 0, STRAIGHT_FORWARD);

      astarsearch->AddSuccessor(NewNode);
    }

    if (GetMap(x_int + 1, y_int) < 9) {
      HybridState4DNode NewNode =
          HybridState4DNode(x_int + 1, y_int, 0, STRAIGHT_FORWARD);

      astarsearch->AddSuccessor(NewNode);
    }

    if (GetMap(x_int, y_int + 1) < 9) {
      HybridState4DNode NewNode =
          HybridState4DNode(x_int, y_int + 1, 0, STRAIGHT_FORWARD);
      astarsearch->AddSuccessor(NewNode);
    }

    return true;
  }  // GetSuccessors

  // (g value) given this node, what does it cost to move to successor.
  // In the case of our map the answer is the map terrain value at this node
  // since that is conceptually where we're moving
  float GetCost(HybridState4DNode &successor) {
    float _G = 0;

    // std::cout << "cost\n";
    // std::cout << SearchConfig_->cost_map << std::endl;

    // auto successor_type = successor.type();

    // switch (successor_type) {
    //   case STRAIGHT_FORWARD:
    //   case LEFTTURN_FORWARD:
    //   case RIGHTTURN_FORWARD: {
    //     if (type_ == successor_type) {
    //       _G = (float)cost_L;
    //     } else {  // turning
    //       if (type_ == STRAIGHT_FORWARD || type_ == LEFTTURN_FORWARD ||
    //           type_ == RIGHTTURN_FORWARD) {  // 进退变化惩罚
    //         _G = dx[0] * Constants::penaltyTurning;
    //       } else {
    //         _G = dx[0] * Constants::penaltyTurning *
    //         Constants::penaltyCOD;
    //       }
    //     }

    //     break;
    //   }
    //   case STRAIGHT_REVERSE:
    //   case LEFTTURN_REVERSE:
    //   case RIGHTTURN_REVERSE: {
    //     break;
    //   }
    //   default:
    //     break;
    // }

    // if (next_type == STRAIGHT_FORWARD || next_type == LEFTTURN_FORWARD
    // ||
    //     type_ == RIGHTTURN_FORWARD) {
    //   if (type !=) } else {
    // }

    // if (prim < 3) {  // 前进代价
    //   // 012前左右
    //   if (pred->prim != prim) {  // 变向惩罚
    //     if (pred->prim > 2) {    // 进退变化惩罚
    //       g += dx[0] * Constants::penaltyTurning *
    //       Constants::penaltyCOD;
    //     } else {
    //       g += dx[0] * Constants::penaltyTurning;
    //     }
    //   } else {
    //     g += dx[0];
    //   }
    // }

    // else {                       // 倒退代价
    //   if (pred->prim != prim) {  // 变向+倒退惩罚
    //     if (pred->prim < 3) {    // 进退变化惩罚
    //       g += dx[0] * Constants::penaltyTurning *
    //       Constants::penaltyReversing *
    //            Constants::penaltyCOD;
    //     } else {
    //       g += dx[0] * Constants::penaltyTurning *
    //       Constants::penaltyReversing;
    //     }
    //   } else {
    //     g += dx[0] * Constants::penaltyReversing;
    //   }
    // }

    return (successor.x() - x_) * (successor.x() - x_) +
           (successor.y() - y_) * (successor.y() - y_);

    // return _G;
  }  // GetCost

  bool IsSameState(HybridState4DNode &rhs) {
    // same state in a maze search is simply when (x,y) are the same
    if (std::hypot(x_ - rhs.x(), y_ - rhs.y()) < 0.1) {
      return true;
    }
    return false;

  }  // IsSameState

  void PrintNodeInfo() {
    char str[100];
    sprintf(str, "Node position : (%f,%f)\n", x_, y_);

    std::cout << str;
  }  // PrintNodeInfo

  static int GetMap(int x, int y) {
    constexpr static int _MAP_WIDTH = 20;
    constexpr static int _MAP_HEIGHT = 30;

    constexpr static int world_map[_MAP_WIDTH * _MAP_HEIGHT] = {

        // 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 00
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 9, 1,  // 01
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 9, 1, 1,  // 02
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 9, 1, 1,  // 03
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 9, 1, 1,  // 04
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1,  // 05
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1,  // 06
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 9, 9, 1,  // 07
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1,  // 08
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 9, 1, 1, 1, 9, 1, 1, 9, 9, 1,  // 09
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1,  // 10
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1,  // 11
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 9, 1, 1,  // 12
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 9, 1, 1,  // 13
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 9, 1, 1,  // 14
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1,  // 15
        1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 9, 9, 9, 1, 1, 1, 1, 1,  // 16
        1, 1, 1, 1, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 1, 1, 9, 9, 9,  // 17
        1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1,  // 18
        1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 9, 1,  // 19
        1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 9, 9, 9, 1,  // 20
        1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1,  // 21
        9, 9, 9, 9, 9, 9, 1, 1, 1, 1, 9, 9, 9, 9, 9, 9, 9, 1, 1, 1,  // 22
        1, 1, 9, 1, 1, 1, 1, 9, 9, 9, 9, 1, 1, 9, 1, 1, 1, 1, 1, 1,  // 23
        1, 1, 9, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 24
        1, 9, 9, 9, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1,  // 25
        1, 1, 9, 1, 1, 1, 1, 9, 9, 9, 9, 9, 9, 9, 9, 1, 1, 1, 9, 1,  // 26
        1, 1, 9, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1,  // 27
        1, 1, 1, 1, 1, 1, 1, 9, 9, 9, 9, 9, 9, 1, 1, 1, 1, 1, 9, 1,  // 28
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 29

    };

    if (x < 0 || x >= _MAP_WIDTH || y < 0 || y >= _MAP_HEIGHT) {
      return 9;
    }

    return world_map[(y * _MAP_WIDTH) + x];
  }  // GetMap

 private:
  float x_;  // the (x,y) positions of the node
  float y_;
  float theta_;
  MovementType type_;

  Eigen::Matrix<float, 3, 6> update_movement_step(float L, float varphi,
                                                  float theta) {
    Eigen::Matrix<float, 3, 6> movement_step =
        Eigen::Matrix<float, 3, 6>::Zero();

    float L_cos_theta = L * std::cos(theta);
    float L_sin_theta = L * std::sin(theta);
    float sin_varphi = std::sin(varphi) / varphi;
    float one_cos_varphi = (1.0 - std::cos(varphi)) / varphi;
    // S+
    movement_step(0, 0) = L_cos_theta;
    movement_step(1, 0) = L_sin_theta;
    movement_step(2, 0) = 0;
    // L+
    movement_step(0, 1) =
        L_cos_theta * sin_varphi - L_sin_theta * one_cos_varphi;
    movement_step(1, 1) =
        L_sin_theta * sin_varphi + L_cos_theta * one_cos_varphi;
    movement_step(2, 1) = varphi;
    // R+
    movement_step(0, 2) =
        L_cos_theta * sin_varphi + L_sin_theta * one_cos_varphi;
    movement_step(1, 2) =
        L_sin_theta * sin_varphi - L_cos_theta * one_cos_varphi;
    movement_step(2, 2) = -varphi;

    for (int i = 3; i != 6; ++i) {
      movement_step(0, i) = -movement_step(0, i - 3);
      movement_step(1, i) = -movement_step(1, i - 3);
      movement_step(2, i) = movement_step(2, i - 3);
    }

    return movement_step;
  }  // update_movement_step

};  // namespace ASV::planning

class HybridAStar {
 public:
  HybridAStar(const CollisionData &collisiondata,
              const HybridAStarConfig &hybridastarconfig)
      : nodeStart_(5, 6, 0),
        nodeEnd_(11, 24, 0),
        collision_checker_(collisiondata),
        rscurve_(3) {
    searchconfig = GenerateSearchConfig(collisiondata, hybridastarconfig);

    astarsearch_.SetStartAndGoalStates(nodeStart_, nodeEnd_);
  }
  virtual ~HybridAStar() = default;

  HybridState4DNode nodeStart() const { return nodeStart_; }
  HybridState4DNode nodeEnd() const { return nodeEnd_; }

  auto results() const { return results_; }

  void performsearch() {
    unsigned int SearchState;
    unsigned int SearchSteps = 0;

    do {
      SearchState = astarsearch_.SearchStep(searchconfig);

      SearchSteps++;

      if constexpr (DEBUG_LISTS == 1) {
        std::cout << "Steps:" << SearchSteps << "\n";

        int len = 0;

        std::cout << "Open:\n";
        HybridState4DNode *p = astarsearch_.GetOpenListStart();
        while (p) {
          len++;
          if constexpr (DEBUG_LIST_LENGTHS_ONLY == 0)
            ((HybridState4DNode *)p)->PrintNodeInfo();
          p = astarsearch_.GetOpenListNext();
        }

        std::cout << "Open list has " << len << " nodes\n";

        len = 0;

        std::cout << "Closed:\n";
        p = astarsearch_.GetClosedListStart();
        while (p) {
          len++;
          if constexpr (DEBUG_LIST_LENGTHS_ONLY == 0) p->PrintNodeInfo();
          p = astarsearch_.GetClosedListNext();
        }

        std::cout << "Closed list has " << len << " nodes\n";
      }

    } while (SearchState ==
             HybridAStarSearch<HybridState4DNode,
                               SearchConfig>::SEARCH_STATE_SEARCHING);

    if (SearchState ==
        HybridAStarSearch<HybridState4DNode,
                          SearchConfig>::SEARCH_STATE_SUCCEEDED) {
      std::cout << "Search found goal state\n";

      HybridState4DNode *node = astarsearch_.GetSolutionStart();

      std::cout << "Displaying solution\n";

      while (node) {
        node->PrintNodeInfo();
        results_.push_back({node->x(), node->y(), node->theta()});
        node = astarsearch_.GetSolutionNext();
      }

      // Once you're done with the solution you can free the nodes up
      astarsearch_.FreeSolutionNodes();
    } else if (SearchState ==
               HybridAStarSearch<HybridState4DNode,
                                 SearchConfig>::SEARCH_STATE_FAILED) {
      std::cout << "Search terminated. Did not find goal state\n";
    }

    // Display the number of loops the search went through
    std::cout << "SearchSteps : " << SearchSteps << "\n";

    astarsearch_.EnsureMemoryFreed();

  }  // performsearch

 private:
  SearchConfig searchconfig;

  HybridState4DNode nodeStart_;
  HybridState4DNode nodeEnd_;
  CollisionChecking collision_checker_;
  ASV::common::math::ReedsSheppStateSpace rscurve_;
  HybridAStarSearch<HybridState4DNode, SearchConfig> astarsearch_;
  std::vector<std::tuple<float, float, float>> results_;

  SearchConfig GenerateSearchConfig(
      const CollisionData &collisiondata,
      const HybridAStarConfig &hybridastarconfig) {
    SearchConfig searchconfig;

    float L = hybridastarconfig.move_length;
    float Ct = hybridastarconfig.penalty_turning;
    float Cr = hybridastarconfig.penalty_reverse;
    float Cs = hybridastarconfig.penalty_switch;

    searchconfig.move_length = L;

    searchconfig.turning_angle = L * collisiondata.MAX_CURVATURE;
    //
    for (int i = 0; i != 3; ++i) {
      for (int j = 0; j != 3; ++j) {
        if (i == j)
          searchconfig.cost_map[i][j] = L;
        else
          searchconfig.cost_map[i][j] = L * Ct;
      }
      for (int j = 3; j != 6; ++j)
        searchconfig.cost_map[i][j] = L * Ct * Cr * Cs;
    }
    for (int i = 3; i != 6; ++i) {
      for (int j = 0; j != 3; ++j) searchconfig.cost_map[i][j] = L * Ct * Cs;
      for (int j = 3; j != 6; ++j) {
        if (i == j)
          searchconfig.cost_map[i][j] = L * Cr;
        else
          searchconfig.cost_map[i][j] = L * Cr * Ct;
      }
    }

    return searchconfig;
  }  // GenerateSearchConfig

};  // end class HybridAStar

}  // namespace ASV::planning

#endif /* _HYBRIDASTAR_H_ */