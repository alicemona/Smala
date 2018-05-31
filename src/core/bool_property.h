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

#include "abstract_property.h"
#include "spike.h"

namespace djnn {
  using namespace std;
  class BoolProperty: public AbstractProperty {
  public:
    BoolProperty (bool v);
    BoolProperty (Process *p, const string &name, bool v);
    bool get_value () { return value; };
    void set_value (int newValue, bool propagate) override;
    void set_value (double v, bool propagate) override;
    void set_value (bool v, bool propagate) override;
    void set_value (Process* v, bool propagate) override;
    void set_value (const std::string &v, bool propagate) override;

    void dump (int level=0) override;
  private:
    bool value;
    std::shared_ptr<Process> _true, _false;
  };
}
