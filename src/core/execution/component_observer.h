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
#include <iostream>
#include <vector>
#include <map>
#include "../process.h"

namespace djnn
{
  using namespace std;
  class ContextManager;

  class ComponentObserver
  {
  public:
    static ComponentObserver& instance ();
    void end_component ();
    void start_component ();
    void end_draw ();
    void start_draw ();
    void add_context_manager (ContextManager *m);
    void remove_context_manager (ContextManager *m);
    void add_draw_context_manager (ContextManager *m);
    void remove_draw_context_manager (ContextManager *m);
    virtual ~ComponentObserver ();
  private:
    ComponentObserver (const ComponentObserver&) = delete;
    ComponentObserver & operator=(const ComponentObserver&) = delete;
    static std::unique_ptr<ComponentObserver> _instance;
    static std::once_flag onceFlag;
    ComponentObserver ();
    vector<ContextManager*> _manager_list;
    vector<ContextManager*> _draw_manager_list;
  };

  class ContextManager {
  public:
    ContextManager () {};
    virtual ~ContextManager () {};
    virtual void pop () = 0;
    virtual void push () = 0;
  };

} /* namespace djnn */

