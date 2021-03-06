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

#include "../execution/graph.h"
#include "process.h"
#include "../control/coupling.h"
#include "../uri.h"
#include "../error.h"
#include <algorithm>
#include <iostream>

namespace djnn
{
  using namespace std;

  int Process::_nb_anonymous = 0;

  void
  alias_children (Process* p, Process* from)
  {
    map<string, Process*> symtable = from->symtable ();
    for (auto& sym : symtable) {
      p->add_symbol (sym.first, sym.second);
    }
  }

  void
  alias (Process *p, const string &name, Process* from)
  {
    p->add_symbol (name, from);
  }

  void
  merge_children (Process *p1, const string &sy1, Process* p2, const string &sy2)
  {
    Process* x2 = p2->find_component (sy2);
    if (x2 == nullptr) {
      cerr << "trying to merge unknown child " << sy2 << endl;
      return;
    }
    Process* x1 = p1->find_component (sy1);
    if (x1 == nullptr) {
      cerr << "trying to merge unknown child " << sy1 << endl;
      return;
    }
    for (auto c : x2->get_activation_couplings ()) {
      x1->add_activation_coupling (c);
    }
    for (auto c : x2->get_deactivation_couplings ()) {
      x1->add_deactivation_coupling (c);
    }
    p2->remove_child (sy2);
    p2->add_symbol (sy2, x1);
    //delete (x2); // hum, are we really sure about this?
  }

  void
  Process::finalize ()
  {
    if (_parent != nullptr)
      _parent->add_child (this, _name);
  }

  Process::Process (Process* parent, const string& name, bool model) :
      _vertex (nullptr), _parent (parent), _state_dependency (nullptr), _source (nullptr), _data (nullptr), _activation_state (
          deactivated), _model (model), _activation_flag (NONE), _has_couplings (false)
  {
    _name = name.length () > 0 ? name : "anonymous_" + to_string (++_nb_anonymous);
    if (_parent != nullptr)
      _state_dependency = _parent->_state_dependency;
    _cpnt_type = UNDEFINED;
    if (Context::instance ()->line () != -1) {
      _dbg_info = std::string ("File: ") + Context::instance ()->filename () + " line: " + std::to_string (Context::instance ()->line ());
    } else
      _dbg_info = "non debug info";
  }

  Process::Process (bool model) :
      _vertex (nullptr), _parent (nullptr), _state_dependency (nullptr), _source (nullptr), _data (nullptr), _activation_state (
          deactivated), _model (model), _activation_flag (NONE), _has_couplings (false)
  {
    _name = "anonymous_" + to_string (++_nb_anonymous);

    _cpnt_type = UNDEFINED;
    if (Context::instance ()->line () != -1) {
      _dbg_info = std::string ("File: ") + Context::instance ()->filename () + " line: " + std::to_string (Context::instance ()->line ());
    } else
      _dbg_info = "non debug info";
  }

  Process::~Process ()
  {
    if (_vertex != nullptr)
      _vertex->invalidate ();
  }

  bool
  Process::is_model ()
  {
    return _model;
  }

  void
  Process::activation ()
  {
    pre_activate ();
    activate ();
    post_activate ();
  }

  void
  Process::deactivation ()
  {
    pre_deactivate ();
    deactivate ();
    post_deactivate ();
  }

  void
  Process::remove_activation_coupling (Coupling* c)
  {
    _activation_couplings.erase (std::remove (_activation_couplings.begin (), _activation_couplings.end (), c),
                                 _activation_couplings.end ());
  }

  void
  Process::remove_deactivation_coupling (Coupling* c)
  {
    _deactivation_couplings.erase (std::remove (_deactivation_couplings.begin (), _deactivation_couplings.end (), c),
                                   _deactivation_couplings.end ());
  }

  Process*
  Process::find_component (const string& key)
  {
    if (key.length () == 0)
      return this;
    size_t found = key.find_first_of ('/');
    if (found != string::npos) {
      string newKey = key.substr (0, found);
      string path = key.substr (found + 1);
      if (newKey[0] == '.' && newKey[1] == '.') {
        if (_parent)
          return _parent->find_component (path);
        else
          return nullptr;
      }
      map<string, Process*>::iterator it = _symtable.find (newKey);
      if (it != _symtable.end ()) {
        return (it->second)->find_component (path);
      }
    }
    if (key[0] == '.' && key[1] == '.')
      return _parent;
    map<string, Process*>::iterator it = _symtable.find (key);
    if (it != _symtable.end ()) {
      return (it->second);
    }
    return 0;
  }

  Process*
  Process::find_component (Process *p, const string &path)
  {
    if (p == nullptr)
      return URI::find_by_uri (path);
    return p->find_component (path);
  }

  string
  Process::find_component_name (Process* symbol)
  {

    // FIXME : low efficiency function cause by linear search. use with care !

    map<string, Process*>::iterator it;
    string key = "name_not_found";

    for (it = _symtable.begin(); it != _symtable.end(); ++it)
    {
      if (it->second == symbol)
      {
        //debug
        //cerr << "key found : " << it->first << endl;
        return it->first;
      }
    }

    return key;
;
  }

  void
  Process::remove_symbol (const string& name)
  {
    map<string, Process*>::iterator it = _symtable.find (name);
    if (it != _symtable.end ())
      _symtable.erase (it);
    else
      cerr << "Warning: symbol " << name << " not found in component " << name << "\n";
  }

  void
  Process::remove_child (Process* c)
  {
    std::map<std::string, Process*>::iterator it;
    for (it = _symtable.begin (); it != _symtable.end (); ++it)
      if (it->second == c) {
        remove_symbol (it->first);
        return;
      }
  }

  void
  Process::remove_child (const string& name)
  {
    remove_symbol (name);
  }

  void
  Process::notify_activation ()
  {
    couplings_t couplings_cpy = couplings_t (_activation_couplings);
    for (auto coupling : couplings_cpy) {
      coupling->propagateActivation ();
    }
  }

  void
  Process::notify_deactivation ()
  {
    couplings_t couplings_cpy = couplings_t (_deactivation_couplings);
    for (auto coupling : couplings_cpy) {
      coupling->propagateDeactivation ();
    }
  }

  void
  Process::set_activation_flag (int flag)
  {
    _activation_flag = flag;
  }

  int
  Process::get_activation_flag ()
  {
    return _activation_flag;
  }

  void
  Process::add_activation_coupling (Coupling* c)
  {
    _activation_couplings.push_back (c);
    _has_couplings = true;
  }

  void
  Process::add_deactivation_coupling (Coupling* c)
  {
    _deactivation_couplings.push_back (c);
    _has_couplings = true;
  }

  void
  Process::add_symbol (const string &name, Process* c)
  {
    /* if ((_symtable.insert (std::pair<string, Process*> (name, c))).second == false) {
     cerr << "Duplicate name " << name << " in component " << _name << endl;
     }*/
    _symtable[name] = c;
  }

  void
  Process::add_child (Process* c, const string& name)
  {
    if (c == nullptr)
      return;
    add_symbol (name, c);
  }

  void
  Process::pre_activate ()
  {
    /* no activation if :
     * 1 - already activated
     * 2 - is activating
     * 3 - the parent exists and is stopped
     */
    if (_activation_state != deactivated || (_parent != 0 && _parent->get_state () > activated))
      return;
    _activation_state = activating;
  }

  void
  Process::post_activate ()
  {
    notify_activation ();
    _activation_state = activated;
  }

  void
  Process::pre_deactivate ()
  {
    if (_activation_state != activated)
      return;
    _activation_state = deactivating;
  }

  void
  Process::post_deactivate ()
  {
    notify_deactivation ();
    _activation_state = deactivated;
    _activation_flag = NONE;
  }

  activation_state
  Process::get_state ()
  {
    return _activation_state;
  }

  Process*
  Process::get_parent ()
  {
    return _parent;
  }

  const string&
  Process::get_name () const
  {
    return _name;
  }

  int
  Process::get_cpnt_type ()
  {
    return _cpnt_type;
  }

  couplings_t
  Process::get_activation_couplings ()
  {
    return _activation_couplings;
  }

  couplings_t
  Process::get_deactivation_couplings ()
  {
    return _deactivation_couplings;
  }

  void
  Process::set_source (Process* src)
  {
    _source = src;
  }

  Process*
  Process::get_activation_source ()
  {
    return _source;
  }

  void
  Process::set_data (Process* data)
  {
    _data = data;
  }

  Process*
  Process::get_data ()
  {
    return _data;
  }

  static int indent = -1;
  void
  Process::dump (int level)
  {
    cout << (_parent ? _parent->find_component_name(this) : _name) << ": ";

    /* check if the component is empty - should be ?*/
    if (_symtable.empty ()) {
      cout << "<EMPTY>" << endl;
      return;
    }

    /* check if the level is reached */
    if ((level != 0) && (indent == level - 1))
      return;

    cout << endl;
    indent++;
    std::map<string, Process*>::iterator it;
    int i = 0;
    for (it = _symtable.begin (); it != _symtable.end (); ++it) {
      for (int j = 0; j < indent; j++)
        cout << "|\t";
      cout << " +" << i++ << " ";
      it->second->dump (level);
      if (it->second->_cpnt_type != COMPONENT || indent == level - 1)
        cout << endl;
    }
    indent--;
  }
}
