//
//  kd_tree.h
//  KDTreeSearch
//
//  Created by Andrew Owens on 2018-05-02.
//  Copyright (c) 2018 ___ALGORITHMIC_BOTANY___. All rights reserved.
//

#pragma once

#include <algorithm>
#include <iterator>

namespace spatial {

template <typename T> constexpr uint16_t dimensions();

template <uint16_t SplitDimensions>
uint16_t nextDimension(uint16_t currentDimension) {
  return (currentDimension + 1) % SplitDimensions;
}

// Inplace "sorting" of a container of points such that the resulting vector
// encodes a kd-tree.
template <uint16_t Dimensions, class RandomIter>
void build_kdTree_inplace(RandomIter first, RandomIter last,
                          uint16_t currentDimension = 0) {
  using std::next;
  using std::distance;
  using point_t = typename std::iterator_traits<RandomIter>::value_type;

  static_assert(Dimensions <= dimensions<point_t>(),
                "[Error] point dimension invalid");

  auto dist = distance(first, last);
  if (dist <= 1)
    return;

  auto mid = next(first, dist / 2);

  // moves the element that would be at position mid if the whole
  // array were sorted to the position mid O(n) complexity
  std::nth_element(first, mid, last,
                   [currentDimension](point_t const &a, point_t const &b) {
                     return a[currentDimension] < b[currentDimension];
                   });

  currentDimension = nextDimension<Dimensions>(currentDimension);

  // left range
  build_kdTree_inplace<Dimensions>(first, mid, currentDimension);

  // right range
  build_kdTree_inplace<Dimensions>(next(mid, 1), last, currentDimension);
}

template <uint16_t Dimensions, class RandomIter, class Float, class Point>
void kdTree_findNeighbours(
    RandomIter first,               // kdTree [first, last)
    RandomIter last,                //
    Point const &p,                 // position
    Float radiusSquared,            // seach radius
    std::vector<Point> &neighbours, // append nearest points
    uint16_t currentDimension = 0   // current dimension
    ) {
  using std::next;
  using std::distance;
  using std::abs;
  using point_t = typename std::iterator_traits<RandomIter>::value_type;

  static_assert(Dimensions <= dimensions<point_t>(),
                "[Error] point dimension invalid");

  auto idxDist = distance(first, last);

  if (idxDist == 0) // empty range
    return;

  if (idxDist == 1) // 1 elemnet range
  {
    auto const &curr = *first;
    auto dist = distanceSquared(curr, p);

    if (dist <= radiusSquared)
      neighbours.emplace_back(curr);
    return;
  }

  // split range in twot
  auto mid = next(first, idxDist / 2);

  // keep point if it is within the radius
  auto const &splitPoint = *mid;

  auto dist = distanceSquared(splitPoint, p);
  if (dist <= radiusSquared)
    neighbours.emplace_back(splitPoint);

  if (p[currentDimension] <= splitPoint[currentDimension]) {
    // recurssive search down "left" split of dimension
    kdTree_findNeighbours<Dimensions>(
        first, mid, // "left" range
        p, radiusSquared, neighbours,
        nextDimension<Dimensions>(currentDimension));

    // if split plane is closer than search radius,
    // need to search otherside as well
    auto lDist = abs(p[currentDimension] - splitPoint[currentDimension]);
    if (lDist * lDist <= radiusSquared) {
      kdTree_findNeighbours<Dimensions>(
          next(mid, 1), last, // "right" range
          p, radiusSquared, neighbours,
          nextDimension<Dimensions>(currentDimension));
    }
  } else {
    // resucrsive search down "right" split of dimension
    kdTree_findNeighbours<Dimensions>(
        next(mid, 1), last, // "right" range
        p, radiusSquared, neighbours,
        nextDimension<Dimensions>(currentDimension));

    // if split plane is closer than search radius,
    // need to search otherside as well
    auto rDist = abs(p[currentDimension] - splitPoint[currentDimension]);
    if (rDist * rDist <= radiusSquared) {
      kdTree_findNeighbours<Dimensions>(
          first, mid, // "left" range
          p, radiusSquared, neighbours,
          nextDimension<Dimensions>(currentDimension));
    }
  }
}

} // namepace spatial