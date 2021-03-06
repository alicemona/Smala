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
 *      Mathieu Poirier <mathieu.poirier@enac.fr>
 *
 */

#pragma once
#include "../tree/process.h"

#include <iostream>

namespace djnn {

  class Exit : public Process
  {
  public:
    Exit (Process* p, const string &name, int value, bool is_model);
    virtual ~Exit ();
    void activate () override;
    void deactivate () override {}
    void serialize (const string& format) override;
  private:
    IntProperty *_value;
  };

}
