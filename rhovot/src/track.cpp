#include "rhovot.hpp"
#include "cmdline.hpp"
#include <cassert>
#include <cmath>

Track::Track()
  : begin_(0),
    end_(CmdlineFactory::TheCmdline().TrackLen())
{
}

TrackPos Track::advance(double speed, TrackPos const& from)
{
    return from + speed;
}

/*static*/ TrackPtr Track::init_tracks() {
    int n_tracks = CmdlineFactory::TheCmdline().NTracks();
    TrackPtr rightmost_track(new Track);
    rightmost_track->num_ = 0;
    TrackPtr prev_track = rightmost_track;
    for (int it=1; it<n_tracks; ++it) {
        TrackPtr curr_track(new Track);
        curr_track->right_ = prev_track;
        curr_track->num_ = it;
        prev_track->left_ = curr_track;
        prev_track = curr_track;
    }
    return rightmost_track;
}
