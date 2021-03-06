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
 *      Mathieu Magnaudet <mathieu.magnaudet@enac.fr>
 *
 */

#pragma once

#include <libudev.h>
#include <linux/input.h>
#include <libevdev/libevdev.h>

#include "../input-priv.h"
#include "../../core/tree/int_property.h"
#include "../../core/tree/set.h"
#include "../../core/syshook/unix/iofd.h"

namespace djnn {
  enum dev_type {
    MOUSE, KEYBOARD, TOUCH_PANEL
  };

#define MT_X (1 << 0)
#define MT_Y (1 << 1)
#define MT_W (1 << 2)
#define MT_H (1 << 3)
#define MT_CX (1 << 4)
#define MT_CY (1 << 5)
#define MT_PRESSURE (1 << 6)

  class LinuxDevice : public Process {
  public:
    LinuxDevice (Process *p, const string& n, dev_type type) : Process (p, n), _type (type) {}
    ~LinuxDevice () {}
    virtual void handle_event (struct input_event *ev) = 0;
    dev_type type () { return _type; }
  private:
    dev_type _type;
  };

  LinuxDevice* map_device (const struct libevdev *_dev, const string &n);

  class Evdev {
    private:
    class EvdevAction: public Process {
      public:
        EvdevAction (Evdev* evdev) :
        Process (), _evdev (evdev) {}
        virtual ~EvdevAction () {}
        void activate ()
        {
          _evdev->handle_evdev_msg ();
        }
        void deactivate () {}
      private:
        Evdev* _evdev;
    };
  public:
    Evdev (const char* node);
    ~Evdev ();
    Process* action () { return _action; }
    void handle_evdev_msg ();
  private:
    Process *_action;
    IOFD *_iofd;
    LinuxDevice *_djn_dev;
    Coupling *_readable_cpl;
    string _name;
    struct libevdev *_dev;
    int _fd;
    bool _aborted;
  };

	class Udev {
  private:
    class UdevAction: public Process {
      public:
        UdevAction (Udev* udev) :
        Process (), _udev (udev) {}
        virtual ~UdevAction () {}
        void activate ()
        {
          _udev->handle_udev_msg ();
        }
        void deactivate () {}
      private:
        Udev* _udev;
    };
  public:
    Udev ();
    ~Udev ();
    void handle_udev_msg ();
    void handle_input_device (struct udev_device*);
  private:
		IOFD* _udev_iofd;
    struct udev *_udev_connection;
    struct udev_monitor *_udev_mon;
    map<string, Evdev*> _sysname_to_dev;
    UdevAction *_action;
    Coupling *_readable_cpl;
	};

  class LinuxMouse : public LinuxDevice
  {
  public:
    LinuxMouse (Process *p, const string &n, const struct libevdev *dev);
    ~LinuxMouse ();
    void mouse_btn_event (const char* name, int val);
    void activate () override {}
    void deactivate () override {}
    void handle_event (struct input_event *ev) override;
  private:
    Spike *_move, *_btn, *_press, *_release, *_wheel;
    IntProperty *_move_dx, *_move_dy, *_wheel_dx, *_wheel_dy;
    TextProperty *_btn_name;
  };

  enum touch_state {
    UNUSED, NEW, USED
  };

  class LinuxTouch : public Process {
    public:
      LinuxTouch (unsigned int fieldmap);
      ~LinuxTouch ();
      void activate () override {}
      void deactivate () override {}
      void set_x (double v) { _x->set_value (v, true); }
      void set_y (double v) { _y->set_value (v, true); }
      void set_width (double v) { _width->set_value (v, true); }
      void set_height (double v) { _height->set_value (v, true); }
      void set_cx (double v) { _cx->set_value (v, true); }
      void set_cy (double v) { _cy->set_value (v, true); }
      void set_pressure (double v) { _pressure->set_value (v, true); }
      touch_state used () { return _used; }
      void set_used (touch_state v) { _used = v; }
    private:
      touch_state _used;
      IntProperty *_x, *_y, *_width, *_height, *_cx, *_cy, *_pressure;
  };

  class LinuxTouchPanel : public LinuxDevice
  {
  public:
    LinuxTouchPanel (Process *p, const string &n, const struct libevdev *dev);
    ~LinuxTouchPanel ();
    void activate () override {}
    void deactivate () override {}
    void handle_event (struct input_event *ev) override;
  private:
    unsigned int _fieldmap;
    int _nb_slots;
    std::vector<LinuxTouch*> _v_touches;
    Set *_touches;
    IntProperty *_max_x, *_max_y;
    LinuxTouch *_cur_touch;
  };
}
