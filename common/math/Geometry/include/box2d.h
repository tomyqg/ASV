/*
***********************************************************************
* box2d.h:  Rectangular (undirected) bounding box in 2-D, where the
*           x/y axes are respectively Forward/Left. For disambiguation,
*           we call the axis of the rectangle parallel to the x-axis
*           direction the "heading-axis". The size of the heading-axis is
*           called "length", and the size of the axis perpendicular to
*           it "width".
*
*  by Hu.ZH(CrossOcean.ai)
***********************************************************************
*/

#ifndef _BOX2D_H_
#define _BOX2D_H_

#include <limits>
#include "aabox2d.h"
#include "linesegment2d.h"

namespace ASV::common::math {

class Box2d {
 public:
  /**
   * @brief Constructor which takes the center, heading, length and width.
   * @param center The center of the rectangular bounding box.
   * @param heading The angle between the x-axis and the heading-axis,
   *        measured counter-clockwise.
   * @param length The size of the heading-axis.
   * @param width The size of the axis perpendicular to the heading-axis.
   */
  explicit Box2d(const Eigen::Vector2d &center, const double heading,
                 const double length, const double width)
      : center_(center),
        length_(length),
        width_(width),
        half_length_(0.5 * length),
        half_width_(0.5 * width),
        heading_(heading),
        cos_heading_(std::cos(heading)),
        sin_heading_(std::sin(heading)),
        max_x_(0.0),
        min_x_(0.0),
        max_y_(0.0),
        min_y_(0.0),
        corners_(Eigen::Matrix<double, 2, 4>::Zero()) {
    InitCorners();
  }

  /**
   * @brief Constructor which takes the heading-axis and the width of the box
   * @param axis The heading-axis
   * @param width The width of the box, which is taken perpendicularly
   * to the heading direction.
   */
  explicit Box2d(const LineSegment2d &axis, const double width)
      : center_(axis.center()),
        length_(axis.length()),
        width_(width),
        half_length_(0.5 * length_),
        half_width_(0.5 * width_),
        heading_(axis.heading()),
        cos_heading_(axis.cos_heading()),
        sin_heading_(axis.sin_heading()),
        max_x_(0.0),
        min_x_(0.0),
        max_y_(0.0),
        min_y_(0.0),
        corners_(Eigen::Matrix<double, 2, 4>::Zero()) {
    InitCorners();
  }

  // Constructor which takes an AABox2d (axes-aligned box).
  explicit Box2d(const AABox2d &aabox)
      : Box2d(aabox.center(), 0, aabox.length(), aabox.width()) {}

  // Getter of the center of the box
  Eigen::Vector2d center() const noexcept { return center_; }

  // Getter of the x/y coordinate of the center of the box
  double center_x() const noexcept { return center_(0); }
  double center_y() const noexcept { return center_(1); }

  double length() const noexcept { return length_; }
  double width() const noexcept { return width_; }
  // Getter of half the length
  double half_length() const noexcept { return half_length_; }

  // Getter of half the width
  double half_width() const noexcept { return half_width_; }

  // Getter of the heading, the counter-clockwise angle between the x-axis and
  // the heading-axis
  double heading() const noexcept { return heading_; }

  // Getter of the cosine of the heading
  double cos_heading() const noexcept { return cos_heading_; }

  // Getter of the sine of the heading
  double sin_heading() const noexcept { return sin_heading_; }

  // Getter of the area of the box
  double area() const noexcept { return length_ * width_; }

  // Getter of the size of the diagonal of the box
  double diagonal() const { return std::hypot(length_, width_); }

  // Getter of the corners of the box
  Eigen::Matrix<double, 2, 4> GetAllCorners() const { return corners_; }

  double max_x() const noexcept { return max_x_; }
  double min_x() const noexcept { return min_x_; }
  double max_y() const noexcept { return max_y_; }
  double min_y() const noexcept { return min_y_; }

  /**
   * @brief Tests points for membership in the box
   * @param point A point that we wish to test for membership in the box
   * @return True iff the point is contained in the box
   */
  bool IsPointIn(const Eigen::Vector2d &point) const {
    auto local_coord = Global2Local(point - center_);
    double dx = std::abs(local_coord(0));
    double dy = std::abs(local_coord(1));

    return dx <= half_length_ + kMathEpsilon &&
           dy <= half_width_ + kMathEpsilon;

  }  // IsPointIn

  /**
   * @brief Tests points for membership in the boundary of the box
   * @param point A point that we wish to test for membership in the boundary
   * @return True iff the point is a boundary point of the box
   */
  bool IsPointOnBoundary(const Eigen::Vector2d &point) const {
    auto local_coord = Global2Local(point - center_);
    double dx = std::abs(local_coord(0));
    double dy = std::abs(local_coord(1));

    return (std::abs(dx - half_length_) <= kMathEpsilon &&
            dy <= half_width_ + kMathEpsilon) ||
           (std::abs(dy - half_width_) <= kMathEpsilon &&
            dx <= half_length_ + kMathEpsilon);
  }  // IsPointOnBoundary

  // Determines the distance between the box and a given point
  double DistanceTo(const Eigen::Vector2d &point) const {
    auto local_coord = Global2Local(point - center_);
    double dx = std::abs(local_coord(0)) - half_length_;
    double dy = std::abs(local_coord(1)) - half_width_;

    if (dx <= 0.0) {
      return std::max(0.0, dy);
    }
    if (dy <= 0.0) {
      return dx;
    }
    return std::hypot(dx, dy);
  }  // DistanceTo

  // Determines the distance between the box and a given line segment
  double DistanceTo(const LineSegment2d &line_segment) const {
    if (line_segment.length() <= kMathEpsilon) {
      return DistanceTo(line_segment.start());
    }
    double box_x = half_length_;
    double box_y = half_width_;

    auto local_coord1 = Global2Local(line_segment.start() - center_);
    double x1 = local_coord1(0);
    double y1 = local_coord1(1);
    int gx1 = (x1 >= box_x ? 1 : (x1 <= -box_x ? -1 : 0));
    int gy1 = (y1 >= box_y ? 1 : (y1 <= -box_y ? -1 : 0));
    if (gx1 == 0 && gy1 == 0) {
      return 0.0;
    }

    auto local_coord2 = Global2Local(line_segment.end() - center_);
    double x2 = local_coord2(0);
    double y2 = local_coord2(1);
    int gx2 = (x2 >= box_x ? 1 : (x2 <= -box_x ? -1 : 0));
    int gy2 = (y2 >= box_y ? 1 : (y2 <= -box_y ? -1 : 0));
    if (gx2 == 0 && gy2 == 0) {
      return 0.0;
    }
    if (gx1 < 0 || (gx1 == 0 && gx2 < 0)) {
      x1 = -x1;
      gx1 = -gx1;
      x2 = -x2;
      gx2 = -gx2;
    }
    if (gy1 < 0 || (gy1 == 0 && gy2 < 0)) {
      y1 = -y1;
      gy1 = -gy1;
      y2 = -y2;
      gy2 = -gy2;
    }
    if (gx1 < gy1 || (gx1 == gy1 && gx2 < gy2)) {
      std::swap(x1, y1);
      std::swap(gx1, gy1);
      std::swap(x2, y2);
      std::swap(gx2, gy2);
      std::swap(box_x, box_y);
    }

    if (gx1 == 1 && gy1 == 1) {
      switch (gx2 * 3 + gy2) {
        case 4:
          return PtSegDistance(box_x, box_y, x1, y1, x2, y2,
                               line_segment.length());
        case 3:
          return (x1 > x2) ? (x2 - box_x)
                           : PtSegDistance(box_x, box_y, x1, y1, x2, y2,
                                           line_segment.length());
        case 2:
          return (x1 > x2) ? PtSegDistance(box_x, -box_y, x1, y1, x2, y2,
                                           line_segment.length())
                           : PtSegDistance(box_x, box_y, x1, y1, x2, y2,
                                           line_segment.length());
        case -1:
          return CrossProd(x2 - x1, y2 - y1, box_x - x1, -box_y - y1) >= 0.0
                     ? 0.0
                     : PtSegDistance(box_x, -box_y, x1, y1, x2, y2,
                                     line_segment.length());
        case -4:
          return CrossProd(x2 - x1, y2 - y1, box_x - x1, -box_y - y1) <= 0.0
                     ? PtSegDistance(box_x, -box_y, x1, y1, x2, y2,
                                     line_segment.length())
                     : (CrossProd(x2 - x1, y2 - y1, -box_x - x1, box_y - y1) <=
                                0.0
                            ? 0.0
                            : PtSegDistance(-box_x, box_y, x1, y1, x2, y2,
                                            line_segment.length()));
      }
    } else {
      switch (gx2 * 3 + gy2) {
        case 4:
          return (x1 < x2) ? (x1 - box_x)
                           : PtSegDistance(box_x, box_y, x1, y1, x2, y2,
                                           line_segment.length());
        case 3:
          return std::min(x1, x2) - box_x;
        case 1:
        case -2:
          return CrossProd(x2 - x1, y2 - y1, box_x - x1, box_y - y1) <= 0.0
                     ? 0.0
                     : PtSegDistance(box_x, box_y, x1, y1, x2, y2,
                                     line_segment.length());
        case -3:
          return 0.0;
      }
    }
    // ACHECK(0) << "unimplemented state: " << gx1 << " " << gy1 << " " << gx2
    //           << " " << gy2;
    return 0.0;

  }  // DistanceTo

  // Determines the distance between two boxes
  double DistanceTo(const Box2d &other_box) const {
    double distance = std::numeric_limits<double>::infinity();

    // if (IsPointIn(polygon.points()[0])) {
    //   return 0.0;
    // }
    // if (polygon.IsPointIn(points_[0])) {
    //   return 0.0;
    // }

    auto other_corners = other_box.GetAllCorners();
    for (int i = 0; i < 3; ++i) {
      distance = std::min(distance,
                          DistanceTo(LineSegment2d(other_corners.col(i),
                                                   other_corners.col(i + 1))));
    }
    distance = std::min(
        distance,
        DistanceTo(LineSegment2d(other_corners.col(3), other_corners.col(0))));
    return distance;
  }

  /**
   * @brief Determines whether this box overlaps a given line segment
   * @param line_segment The line-segment
   * @return True if they overlap
   */
  bool HasOverlap(const LineSegment2d &line_segment) const {
    if (line_segment.length() <= kMathEpsilon) {
      return IsPointIn(line_segment.start());
    }
    auto p_start = line_segment.start();
    auto p_end = line_segment.end();

    if ((std::fmax(p_start(0), p_end(0)) < min_x()) ||
        (std::fmin(p_start(0), p_end(0)) > max_x()) ||
        (std::fmax(p_start(1), p_end(1)) < min_y()) ||
        (std::fmin(p_start(1), p_end(1)) > max_y())) {
      return false;
    }
    return DistanceTo(line_segment) <= kMathEpsilon;
  }  // HasOverlap

  /**
   * @brief Determines whether these two boxes overlap
   * @param line_segment The other box
   * @return True if they overlap
   */
  bool HasOverlap(const Box2d &box) const {
    if (box.max_x() < min_x() || box.min_x() > max_x() ||
        box.max_y() < min_y() || box.min_y() > max_y()) {
      return false;
    }

    const double shift_x = box.center_x() - center_(0);
    const double shift_y = box.center_y() - center_(1);

    const double dx1 = cos_heading_ * half_length_;
    const double dy1 = sin_heading_ * half_length_;
    const double dx2 = sin_heading_ * half_width_;
    const double dy2 = -cos_heading_ * half_width_;
    const double dx3 = box.cos_heading() * box.half_length();
    const double dy3 = box.sin_heading() * box.half_length();
    const double dx4 = box.sin_heading() * box.half_width();
    const double dy4 = -box.cos_heading() * box.half_width();

    return std::abs(shift_x * cos_heading_ + shift_y * sin_heading_) <=
               std::abs(dx3 * cos_heading_ + dy3 * sin_heading_) +
                   std::abs(dx4 * cos_heading_ + dy4 * sin_heading_) +
                   half_length_ &&
           std::abs(shift_x * sin_heading_ - shift_y * cos_heading_) <=
               std::abs(dx3 * sin_heading_ - dy3 * cos_heading_) +
                   std::abs(dx4 * sin_heading_ - dy4 * cos_heading_) +
                   half_width_ &&
           std::abs(shift_x * box.cos_heading() +
                    shift_y * box.sin_heading()) <=
               std::abs(dx1 * box.cos_heading() + dy1 * box.sin_heading()) +
                   std::abs(dx2 * box.cos_heading() + dy2 * box.sin_heading()) +
                   box.half_length() &&
           std::abs(shift_x * box.sin_heading() -
                    shift_y * box.cos_heading()) <=
               std::abs(dx1 * box.sin_heading() - dy1 * box.cos_heading()) +
                   std::abs(dx2 * box.sin_heading() - dy2 * box.cos_heading()) +
                   box.half_width();
  }  // HasOverlap

  // Gets the smallest axes-aligned box containing the current one
  AABox2d GetSmallestAABox() const {
    const double dx1 = std::abs(cos_heading_ * half_length_);
    const double dy1 = std::abs(sin_heading_ * half_length_);
    const double dx2 = std::abs(sin_heading_ * half_width_);
    const double dy2 = std::abs(cos_heading_ * half_width_);
    return AABox2d(center_, (dx1 + dx2) * 2.0, (dy1 + dy2) * 2.0);
  }  // GetAABox

  // Rotate from center.
  void RotateFromCenter(const double rotate_angle) {
    heading_ = Normalizeheadingangle(heading_ + rotate_angle);
    cos_heading_ = std::cos(heading_);
    sin_heading_ = std::sin(heading_);
    InitCorners();

  }  // RotateFromCenter

  // Shifts this box by a given vector
  void Shift(const Eigen::Vector2d &shift_vec) {
    center_ += shift_vec;
    InitCorners();
  }  // Shift

  /**
   * @brief Extend the box longitudinally
   * @param extension_length the length to extend
   */
  void LongitudinalExtend(const double extension_length) {
    length_ += extension_length;
    half_length_ = 0.5 * length_;
    InitCorners();
  }  // LongitudinalExtend

  void LateralExtend(const double extension_width) {
    width_ += extension_width;
    half_width_ = 0.5 * width_;
    InitCorners();
  }  // LateralExtend

 private:
  Eigen::Vector2d center_;
  double length_;
  double width_;
  double half_length_;
  double half_width_;
  double heading_;
  double cos_heading_;
  double sin_heading_;

  double max_x_;
  double min_x_;
  double max_y_;
  double min_y_;
  Eigen::Matrix<double, 2, 4> corners_;

  void InitCorners() {
    corners_.col(0) =
        center_ +
        Local2Global(
            (Eigen::Vector2d() << half_length_, -half_width_).finished());
    corners_.col(1) =
        center_ +
        Local2Global(
            (Eigen::Vector2d() << half_length_, half_width_).finished());
    corners_.col(2) =
        center_ +
        Local2Global(
            (Eigen::Vector2d() << -half_length_, half_width_).finished());
    corners_.col(3) =
        center_ +
        Local2Global(
            (Eigen::Vector2d() << -half_length_, -half_width_).finished());

    max_x_ = corners_.row(0).maxCoeff();
    min_x_ = corners_.row(0).minCoeff();
    max_y_ = corners_.row(1).maxCoeff();
    min_y_ = corners_.row(1).minCoeff();

  }  // InitCorners

  Eigen::Vector2d Local2Global(const Eigen::Vector2d &local_coord) const {
    Eigen::Vector2d global_coor = Eigen::Vector2d::Zero();

    global_coor << local_coord(0) * cos_heading_ -
                       local_coord(1) * sin_heading_,
        local_coord(0) * sin_heading_ + local_coord(1) * cos_heading_;
    return global_coor;
  }  // Local2Global

  Eigen::Vector2d Global2Local(const Eigen::Vector2d &global_coord) const {
    Eigen::Vector2d local_coor = Eigen::Vector2d::Zero();

    local_coor << global_coord(0) * cos_heading_ +
                      global_coord(1) * sin_heading_,
        -global_coord(0) * sin_heading_ + global_coord(1) * cos_heading_;
    return local_coor;
  }  // Global2Local

  double PtSegDistance(double query_x, double query_y, double start_x,
                       double start_y, double end_x, double end_y,
                       double length) const {
    const double x0 = query_x - start_x;
    const double y0 = query_y - start_y;
    const double dx = end_x - start_x;
    const double dy = end_y - start_y;

    double proj = InnerProd(x0, y0, dx, dy);

    if (proj <= 0.0) {
      return std::hypot(x0, y0);
    }
    if (proj >= length * length) {
      return std::hypot(x0 - dx, y0 - dy);
    }

    return std::abs(CrossProd(x0, y0, dx, dy)) / length;
  }  // PtSegDistance

};  // end class Box2d

}  // namespace ASV::common::math

#endif /* _BOX2D_H_ */