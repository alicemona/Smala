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

#include "geometry.h"
#include "../core/serializer/serializer.h"

namespace djnn
{
  HermiteCurve::HermiteCurveAction::HermiteCurveAction (Process* parent, const string &name,
                                                        AbstractProperty *input, AbstractProperty *p1, AbstractProperty *p2,
                                                        AbstractProperty *t1, AbstractProperty *t2, AbstractProperty *output) :
      Process (parent, name), _input (input), _p1 (p1), _p2 (p2), _t1 (t1), _t2 (t2), _output (output)
  {
    Process::finalize ();
  }

  void
  HermiteCurve::HermiteCurveAction::activate ()
  {
    if (_parent->get_state () > activated)
      return;
    double p1 = ((DoubleProperty*) _p1)->get_value ();
    double p2 = ((DoubleProperty*) _p2)->get_value ();
    double t1 = ((DoubleProperty*) _t1)->get_value ();
    double t2 = ((DoubleProperty*) _t2)->get_value ();
    double in = ((DoubleProperty*) _input)->get_value ();
    double pow2 = in * in;
    double pow3 = pow2 * in;
    double h1 = 2 * pow3 - 3 * pow2 + 1;
    double h2 = -2 * pow3 + 3 * pow2;
    double h3 = pow3 - 2 * pow2 + in;
    double h4 = pow3 - pow2;
    double out = h1 * p1 + h2 * p2 + h3 * t1 + h4 * t2;
    _output->set_value (out, true);
  }

  HermiteCurve::HermiteCurve (Process *p, const string &n, double p1, double p2, double t1, double t2) :
      Process (p, n)
  {
    _input = new DoubleProperty (this, "input", 0);
    _p1 = new DoubleProperty (this, "p1", p1);
    _p2 = new DoubleProperty (this, "p2", p2);
    _t1 = new DoubleProperty (this, "t1", t1);
    _t2 = new DoubleProperty (this, "t2", t2);
    _output = new DoubleProperty (this, "output", 0);
    _action = new HermiteCurveAction (this, "action", _input, _p1, _p2, _t1, _t2, _output);
    _c_input = new Coupling (_input, ACTIVATION, _action, ACTIVATION);
    _c_input->disable ();
    _c_p1 = new Coupling (_p1, ACTIVATION, _action, ACTIVATION);
    _c_p1->disable ();
    _c_p2 = new Coupling (_p2, ACTIVATION, _action, ACTIVATION);
    _c_p2->disable ();
    _c_t1 = new Coupling (_t1, ACTIVATION, _action, ACTIVATION);
    _c_t1->disable ();
    _c_t2 = new Coupling (_t2, ACTIVATION, _action, ACTIVATION);
    _c_t2->disable ();
    Graph::instance ().add_edge (_input, _action);
    Graph::instance ().add_edge (_p1, _action);
    Graph::instance ().add_edge (_p2, _action);
    Graph::instance ().add_edge (_t1, _action);
    Graph::instance ().add_edge (_t2, _action);
    Graph::instance ().add_edge (_action, _output);
    if (_parent && _parent->state_dependency () != nullptr)
      Graph::instance ().add_edge (_parent->state_dependency (), _action);
    Process::finalize ();
  }

  HermiteCurve::~HermiteCurve ()
  {
    if (_parent && _parent->state_dependency () != nullptr)
      Graph::instance ().remove_edge (_parent->state_dependency (), _action);
    Graph::instance ().remove_edge (_input, _action);
    Graph::instance ().remove_edge (_p1, _action);
    Graph::instance ().remove_edge (_p2, _action);
    Graph::instance ().remove_edge (_t1, _action);
    Graph::instance ().remove_edge (_t2, _action);
    Graph::instance ().remove_edge (_action, _output);

    if (_c_t2) { delete _c_t2; _c_t2 = nullptr;}
    if (_c_t1) { delete _c_t1; _c_t1 = nullptr;}
    if (_c_p2) { delete _c_p2; _c_p2 = nullptr;}
    if (_c_p1) { delete _c_p1; _c_p1 = nullptr;}
    if (_c_input) { delete _c_input; _c_input = nullptr;}
    if (_action) { delete _action; _action = nullptr;}
    if (_output) { delete _output; _output = nullptr;}
    if (_t2) { delete _t2; _t2 = nullptr;}
    if (_t1) { delete _t1; _t1 = nullptr;}
    if (_p2) { delete _p2; _p2 = nullptr;}
    if (_p1) { delete _p1; _p1 = nullptr;}
    if (_input) { delete _input; _input = nullptr;}
  }

  void
  HermiteCurve::activate ()
  {
    _c_input->enable ();
    _c_p1->enable ();
    _c_p2->enable ();
    _c_t1->enable ();
    _c_t2->enable ();
  }

  void
  HermiteCurve::deactivate ()
  {
    _c_input->disable ();
    _c_p1->disable ();
    _c_p2->disable ();
    _c_t1->disable ();
    _c_t2->disable ();
  }

  void
  HermiteCurve::serialize (const string& type) {
   
    AbstractSerializer::pre_serialize(this, type);

    AbstractSerializer::serializer->start ("base:hermitecurve");
    AbstractSerializer::serializer->text_attribute ("id", _name);
    AbstractSerializer::serializer->float_attribute ("p1", dynamic_cast<DoubleProperty*> (_p1)->get_value ());
    AbstractSerializer::serializer->float_attribute ("p2", dynamic_cast<DoubleProperty*> (_p2)->get_value ());
    AbstractSerializer::serializer->float_attribute ("t1", dynamic_cast<DoubleProperty*> (_t1)->get_value ());
    AbstractSerializer::serializer->float_attribute ("t2", dynamic_cast<DoubleProperty*> (_t2)->get_value ());

    AbstractSerializer::serializer->end ();

    AbstractSerializer::post_serialize(this);

  }
}

