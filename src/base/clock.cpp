/*
 *  djnn v2
 *
 *  The copyright holders for the contents of this file are:
 *      Ecole Nationale de l'Aviation Civile, France (2018)
 *  See file "license.terms" for the rights and conditions
 *  defined by copyright holders.
 *
 *
 *  Contributors:
 *      Stéphane Conversy <stephane.conversy@enac.fr>
 *      Mathieu Magnaudet <mathieu.magnaudet@enac.fr>
 *
 */

#include "clock.h"
#include "../core/syshook/syshook.h"
#include "../core/tree/int_property.h"
#include "../core/execution/graph.h"
#include "../core/serializer/serializer.h"
#include "../core/utils-dev.h"
#include <sys/time.h>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

//#include <QThread.h>
#include <iostream>

#define DBG std::cerr << __FILE__ ":" << __LINE__ << ":" << __FUNCTION__ << std::endl;

namespace djnn
{
  Clock::Clock (std::chrono::milliseconds period)
  {
    _period = new IntProperty (this, "period", period.count ());
    _elapsed = new DoubleProperty (this, "elapsed", 0);
    _tick = new Spike (this, "tick");
  }

  Clock::Clock (Process *p, const std::string& n, std::chrono::milliseconds period) :
      Process (p, n)
  {
    _period = new IntProperty (this, "period", period.count ());
    _elapsed = new DoubleProperty (this, "elapsed", 0);
    _tick = new Spike (this, "tick");
    Process::finalize ();
  }

  Clock::Clock (int period)
  {
    _period = new IntProperty (this, "period", period);
    _elapsed = new DoubleProperty (this, "elapsed", 0);
    _tick = new Spike (this, "tick");
  }

  Clock::Clock (Process *p, const std::string& n, int period) :
      Process (p, n)
  {
    _period = new IntProperty (this, "period", period);
    _elapsed = new DoubleProperty (this, "elapsed", 0);
    _tick = new Spike (this, "tick");
    Process::finalize ();
  }

  Clock::~Clock ()
  {
    if (_tick) { delete _tick; _tick = nullptr;}
    if (_elapsed) { delete _elapsed; _elapsed = nullptr;}
    if (_period) { delete _period; _period = nullptr;}
  }

  void
  Clock::activate ()
  {
    //djnn::get_exclusive_access (DBG_GET); // no break after this call without release !!
    //set_please_stop (true);
    //if ( _thread.joinable() ) _thread.join();
    //DBG;
    please_stop();
    start_thread();

    //djnn::release_exclusive_access (DBG_REL); // no break before this call without release !!
  }

  void
  Clock::deactivate ()
  {
    please_stop ();
  }

  void
  Clock::run ()
  {
//    QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);
    struct timespec before;
    struct timespec after;
    set_please_stop (false);
    try {
      //std::cerr << this << " >> run" << std::endl;
      while (!get_please_stop ()) {
        //std
        boost
        ::chrono::milliseconds duration (_period->get_value ());
        //std::cerr << this << "  >> sleep " << duration.count() << std::endl;
        //std
        get_monotonic_time(&before);
        boost
        ::this_thread::sleep_for (duration); // blocking call
        //std::cerr << this << "  << sleep end" << std::endl;
        djnn::get_exclusive_access (DBG_GET); // no break after this call without release !!
        //std::cerr << this << "  ** sleep GOT" << std::endl;
        if (!get_please_stop ()) {
          get_monotonic_time(&after);
          double elapsedTime = (after.tv_sec * 1000 + after.tv_nsec * 1e-6) - (before.tv_sec * 1000 + before.tv_nsec * 1e-6);
          _elapsed->set_value (elapsedTime, true);
          _tick->activation (); // propagating
          GRAPH_EXEC; // executing
        }
        djnn::release_exclusive_access (DBG_REL); // no break before this call without release !!
      }
      //std::cerr << this << " << stop" << std::endl;

    } catch (exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }

  void
  Clock::serialize (const string& type) {
   
    AbstractSerializer::pre_serialize(this, type);

    AbstractSerializer::serializer->start ("base:clock");
    AbstractSerializer::serializer->text_attribute ("id", _name);
    AbstractSerializer::serializer->int_attribute ("period", _period->get_value ());
    AbstractSerializer::serializer->end ();

    AbstractSerializer::post_serialize(this);

  }

}









