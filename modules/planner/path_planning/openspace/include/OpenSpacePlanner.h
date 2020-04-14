/*
*******************************************************************************
* OpenSpacePlanner.h:
* Path planner used in the low-speed vessel, including hybrid A star,
* trajectory smoother and collision checking. This planner is designed to
* be used in both fully-actuated and underactuated vessels.
* This header file can be read by C++ compilers
*
* by Hu.ZH(CrossOcean.ai)
*******************************************************************************
*/

#ifndef _OPENSPACEPLANNER_H_
#define _OPENSPACEPLANNER_H_

#include "HybridAStar.h"
#include "PathSmoothing.h"

namespace ASV::planning {

class OpenSpacePlanner {
 public:
  OpenSpacePlanner(const CollisionData &collisiondata,
                   const HybridAStarConfig &hybridastarconfig,
                   const SmootherConfig &smootherconfig)
      : collision_checker_(collisiondata),
        Hybrid_AStar_(collisiondata, hybridastarconfig),
        path_smoother_(smootherconfig) {}
  virtual ~OpenSpacePlanner() = default;

  OpenSpacePlanner &update_obstacles(
      const std::vector<Obstacle_Vertex_Config> &Obstacles_Vertex,
      const std::vector<Obstacle_LineSegment_Config> &Obstacles_LineSegment,
      const std::vector<Obstacle_Box2d_Config> &Obstacles_Box2d) {
    collision_checker_.set_all_obstacls(Obstacles_Vertex, Obstacles_LineSegment,
                                        Obstacles_Box2d);

    return *this;
  }  // update_obstacles

  OpenSpacePlanner &update_start_end(const std::array<double, 3> start_point,
                                     const std::array<double, 3> end_point) {
    Hybrid_AStar_.setup_start_end(static_cast<float>(start_point.at(0)),
                                  static_cast<float>(start_point.at(1)),
                                  static_cast<float>(start_point.at(2)),
                                  static_cast<float>(end_point.at(0)),
                                  static_cast<float>(end_point.at(1)),
                                  static_cast<float>(end_point.at(2)));
    return *this;
  }  // update_start_end

  OpenSpacePlanner &GenerateTrajectory() {
    Hybrid_AStar_.perform_4dnode_search(collision_checker_);
    auto coarse_path_direction = Hybrid_AStar_.hybridastar_trajecotry();

    std::cout << "coarse path direction\n";
    for (const auto &segment : coarse_path_direction) {
      std::cout << std::get<0>(segment) << ", " << std::get<1>(segment) << ", "
                << std::get<2>(segment) << ", " << std::get<3>(segment)
                << std::endl;
    }
    coarse_path_.clear();
    for (const auto &value : coarse_path_direction)
      coarse_path_.push_back(
          {std::get<0>(value), std::get<1>(value), std::get<2>(value)});

    path_smoother_.SetupCoarsePath(coarse_path_direction);
    auto p_smooth_path =
        path_smoother_.PerformSmoothing(collision_checker_).smooth_path();

    fine_path_.clear();
    for (const auto &segment : p_smooth_path) {
      std::cout << "one segment\n";
      for (const auto &state : segment) {
        std::cout << state.x() << ", " << state.y() << std::endl;

        fine_path_.push_back({state.x(), state.y(), 0});
      }
    }
    return *this;

  }  // GenerateTrajectory

  std::vector<std::array<double, 3>> coarse_path() const {
    return coarse_path_;
  }
  std::vector<std::array<double, 3>> fine_path() const { return fine_path_; }

 private:
  CollisionChecking_Astar collision_checker_;
  HybridAStar Hybrid_AStar_;
  PathSmoothing path_smoother_;

  std::vector<std::array<double, 3>> coarse_path_;
  std::vector<std::array<double, 3>> fine_path_;

};  // end class OpenSpacePlanner

}  // namespace ASV::planning

#endif /* _OPENSPACEPLANNER_H_ */