/*
****************************************************************************
* TargetTracking.h:
* Target tracking using marine radar
* This header file can be read by C++ compilers
*
* by Hu.ZH(CrossOcean.ai)
****************************************************************************
*/

#ifndef _TARGETTRACKING_H_
#define _TARGETTRACKING_H_

#include <pyclustering/cluster/dbscan.hpp>
#include <pyclustering/utils/metric.hpp>
#include "SpokeProcessingData.h"
#include "common/math/Geometry/include/Miniball.hpp"

#include "common/timer/include/timecounter.h"

namespace ASV::perception {
class TargetTracking {
 public:
  TargetTracking(const AlphaBetaData &_AlphaBeta_data,
                 const ClusteringData &_ClusteringData)
      : AlphaBeta_data(_AlphaBeta_data), Clustering_data(_ClusteringData) {}
  virtual ~TargetTracking() = default;

  TargetTracking &AutoTracking(const std::vector<double> &_surroundings_x,
                               const std::vector<double> &_surroundings_y) {
    ClusteringAndMiniBall(_surroundings_x, _surroundings_y,
                          TargetTracker_RTdata.target_x,
                          TargetTracker_RTdata.target_y,
                          TargetTracker_RTdata.target_square_radius);
    auto [postion_x, postion_y, velocity_vx, velocity_y] =
        AlphaBetaFiltering(1, 1, 1, 1, 1, 1);
    return *this;
  }  // AutoTracking

  TargetTrackerRTdata getTargetTrackerRTdata() const noexcept {
    return TargetTracker_RTdata;
  }

 private:
  const AlphaBetaData AlphaBeta_data;
  const ClusteringData Clustering_data;
  TargetTrackerRTdata TargetTracker_RTdata;

  // clustering for all points and find the miniball around each sets of points
  void ClusteringAndMiniBall(const std::vector<double> &_surroundings_x,
                             const std::vector<double> &_surroundings_y,
                             std::vector<double> &_target_x,
                             std::vector<double> &_target_y,
                             std::vector<double> &_target_radius) {
    // clustering for all points
    std::shared_ptr<pyclustering::dataset> p_data =
        std::make_shared<pyclustering::dataset>();
    std::shared_ptr<pyclustering::clst::dbscan_data> ptr_output_result =
        std::make_shared<pyclustering::clst::dbscan_data>();

    std::size_t num_surroundings = _surroundings_x.size();
    p_data->resize(num_surroundings);
    for (std::size_t i = 0; i != num_surroundings; ++i) {
      p_data->at(i) = {_surroundings_x[i], _surroundings_y[i]};
    }

    pyclustering::clst::dbscan clustering_solver(
        Clustering_data.p_radius, Clustering_data.p_minumum_neighbors);
    clustering_solver.process(*p_data, *ptr_output_result);
    const pyclustering::clst::cluster_sequence &actual_clusters =
        ptr_output_result->clusters();

    std::size_t num_actual_clusters = actual_clusters.size();

    // miniball for each cluster
    _target_x.resize(num_actual_clusters);
    _target_y.resize(num_actual_clusters);
    _target_radius.resize(num_actual_clusters);

    for (std::size_t index = 0; index != num_actual_clusters; ++index) {
      auto cluster = actual_clusters.at(index);

      int d = 2;                       // dimension
      std::size_t n = cluster.size();  // number of points

      double **ap = new double *[n];
      for (std::size_t j = 0; j < n; ++j) {
        double *p = new double[d];
        p[0] = _surroundings_x[cluster.at(j)];
        p[1] = _surroundings_y[cluster.at(j)];
        ap[j] = p;
      }
      // create an instance of Miniball
      Miniball::Miniball<
          Miniball::CoordAccessor<double *const *, const double *> >
          mb(d, ap, ap + n);

      // output results
      const double *center = mb.center();
      _target_x[index] = center[0];
      _target_y[index] = center[1];
      _target_radius[index] = mb.squared_radius();

      // clean up
      for (std::size_t j = 0; j < n; ++j) delete[] ap[j];
      delete[] ap;
    }

  }  // ClusteringAndMiniBall

  // alpha-beta filtering for 2d target tracking
  std::tuple<double, double, double, double> AlphaBetaFiltering(
      const double previous_target_x, const double previous_target_y,
      const double previous_target_vx, const double previous_target_vy,
      const double meas_x, const double meas_y) {
    double previous_position[2] = {previous_target_x, previous_target_y};
    double previous_velocity[2] = {previous_target_vx, previous_target_vy};
    double measurement[2] = {meas_x, meas_y};
    double position[2] = {};
    double velocity[2] = {};

    for (int i = 0; i != 2; ++i) {
      double xk = previous_position[i] +
                  AlphaBeta_data.sample_time * previous_velocity[i];
      double rk = measurement[i] - xk;
      position[i] = xk + AlphaBeta_data.alpha * rk;
      velocity[i] = previous_velocity[i] +
                    rk * AlphaBeta_data.beta / AlphaBeta_data.sample_time;
    }
    return {position[0], position[1], velocity[0], velocity[1]};
  }  // AlphaBetaFiltering

};  // end class TargetTracking

}  // namespace ASV::perception

#endif /* _TARGETTRACKING_H_ */