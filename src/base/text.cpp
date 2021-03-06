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

#include "../core/tree/text_property.h"
#include "../core/serializer/serializer.h"
#include "text.h"

namespace djnn
{

  void
  TextPrinter::init ()
  {
    _input = new TextProperty (this, "input", "");
    _action = new TextPrinterAction (this, get_name () + "_action", _input);
    c_input = new Coupling (_input, ACTIVATION, _action, ACTIVATION);
    c_input->disable ();
    Graph::instance ().add_edge (_input, _action);
    if (_parent && _parent->state_dependency () != nullptr)
      Graph::instance ().add_edge (_parent->state_dependency (), _action);
  }

  TextPrinter::~TextPrinter ()
  {
    if (_parent && _parent->state_dependency () != nullptr)
      Graph::instance ().remove_edge (_parent->state_dependency (), _action);
    Graph::instance ().remove_edge (_input, _action);

    if (c_input) { delete c_input; c_input=nullptr;}
    if (_action) { delete _action; _action=nullptr;}
    if (_input) { delete _input; _input=nullptr;}  
  }

  void
  TextPrinter::TextPrinterAction::activate ()
  {
    std::cout << _input->get_value () << std::endl;
  }

  TextPrinter::TextPrinter (Process *p, const string &n) :
      Process (p, n)
  {
    init ();
    Process::finalize ();
  }

  TextPrinter::TextPrinter ()
  {
    init ();
  }

  void
  TextPrinter::serialize (const string& type) {
   
    AbstractSerializer::pre_serialize(this, type);

    AbstractSerializer::serializer->start ("base:textprinter");
    AbstractSerializer::serializer->text_attribute ("id", _name);
   
    AbstractSerializer::serializer->end ();

    AbstractSerializer::post_serialize(this);
  }

  TextCatenator::TextCatenator (Process *p, const string &n) :
      BinaryOperator (p, n)
  {
    _left = new TextProperty (this, "head", "");
    _right = new TextProperty (this, "tail", "");
    _result = new TextProperty (this, "output", "");
    init_couplings (new TextCatenatorAction (this, n + "_action", _left, _right, _result));
    Process::finalize ();
  }

  void
  TextCatenator::serialize (const string& type) {
   
    AbstractSerializer::pre_serialize(this, type);

    AbstractSerializer::serializer->start ("base:textcatenator");
    AbstractSerializer::serializer->text_attribute ("id", _name);
   
    AbstractSerializer::serializer->end ();

    AbstractSerializer::post_serialize(this);
  }

  TextComparator::TextComparator (Process *p, const string &n, const string &left, const string &right) :
      BinaryOperator (p, n)
  {
    _left = new TextProperty (this, "left", left);
    _right = new TextProperty (this, "right", right);
    _result = new BoolProperty (this, "output", left.compare (right) == 0);
    init_couplings (new TextComparatorAction (this, n + "_action", _left, _right, _result));
    Process::finalize ();
  }

  void
  TextComparator::serialize (const string& type) {
   
    AbstractSerializer::pre_serialize(this, type);

    AbstractSerializer::serializer->start ("base:textcomparator");
    AbstractSerializer::serializer->text_attribute ("id", _name);
    AbstractSerializer::serializer->text_attribute ("left", dynamic_cast<TextProperty*> (_left)->get_value ());
    AbstractSerializer::serializer->text_attribute ("right", dynamic_cast<TextProperty*> (_right)->get_value ());
    
    AbstractSerializer::serializer->end ();

    AbstractSerializer::post_serialize(this);
  }

  void
  DoubleFormatter::init (double initial, int decimal)
  {
    _input = new DoubleProperty (this, "input", initial);
    _decimal = new IntProperty (this, "decimal", decimal);
    _output = new TextProperty (this, "output", "");
    _action = new DoubleFormatterAction (this, "action", _input, _decimal, _output);
    _c_input = new Coupling (_input, ACTIVATION, _action, ACTIVATION);
    _c_input->disable ();
    _c_decimal = new Coupling (_decimal, ACTIVATION, _action, ACTIVATION);
    _c_decimal->disable ();
    Graph::instance ().add_edge (_input, _action);
    Graph::instance ().add_edge (_decimal, _action);
    Graph::instance ().add_edge (_action, _output);
    if (_parent && _parent->state_dependency () != nullptr)
      Graph::instance ().add_edge (_parent->state_dependency (), _action);
  }

  DoubleFormatter::~DoubleFormatter ()
  {
    if (_parent && _parent->state_dependency () != nullptr)
      Graph::instance ().remove_edge (_parent->state_dependency (), _action);
    Graph::instance ().remove_edge (_input, _action);
    Graph::instance ().remove_edge (_decimal, _action);
    Graph::instance ().remove_edge (_action, _output);

    if (_c_decimal) { delete _c_decimal; _c_decimal=nullptr;}
    if (_c_input) { delete _c_input; _c_input=nullptr;}
    if (_action) { delete _action; _action=nullptr;}
    if (_output) { delete _output; _output=nullptr;}
    if (_decimal) { delete _decimal; _decimal=nullptr;}
    if (_input) { delete _input; _input=nullptr;}
    
  }
  DoubleFormatter::DoubleFormatter (Process* parent, const string &name, double initial, int decimal) :
      Process (parent, name)
  {
    init (initial, decimal);
    Process::finalize ();
  }

  DoubleFormatter::DoubleFormatter (double initial, int decimal) :
      Process ()
  {
    init (initial, decimal);
  }

  void
  DoubleFormatter::activate ()
  {
    _c_input->enable ();
    _c_decimal->enable ();
    _action->activation ();
  }

  void
  DoubleFormatter::deactivate ()
  {
    _c_input->disable ();
    _c_decimal->disable ();
    _action->deactivation ();
  }

  void
  DoubleFormatter::serialize (const string& type) {
   
    AbstractSerializer::pre_serialize(this, type);

    AbstractSerializer::serializer->start ("base:doubleformatter");
    AbstractSerializer::serializer->text_attribute ("id", _name);
    AbstractSerializer::serializer->float_attribute ("initial", dynamic_cast<DoubleProperty*> (_input)->get_value ());
    AbstractSerializer::serializer->int_attribute ("decimal", dynamic_cast<IntProperty*> (_decimal)->get_value ());
    
    AbstractSerializer::serializer->end ();

    AbstractSerializer::post_serialize(this);
  }

  TextAccumulator::TextAccumulator (Process *p, const string &n, const string &init) : Process (p, n)
  {
    _input = new TextProperty (this, "input", "");
    _state = new TextProperty (this, "state", init);
    _del = new Spike (this, "delete");
    _acc_action = new AccumulateAction (this, "acc_action", _input, _state);
    _del_action = new DeleteAction (this, "del_action", _state);
    _c_acc = new Coupling (_input, ACTIVATION, _acc_action, ACTIVATION);
    _c_acc->disable ();
    _c_del = new Coupling (_del, ACTIVATION, _del_action, ACTIVATION);
    _c_del->disable ();
    Graph::instance ().add_edge (_input, _acc_action);
    Graph::instance ().add_edge (_del, _del_action);
    Graph::instance ().add_edge (_acc_action, _state);
    Graph::instance ().add_edge (_del_action, _state);
    if (_parent && _parent->state_dependency () != nullptr) {
      Graph::instance ().add_edge (_parent->state_dependency (), _acc_action);
      Graph::instance ().add_edge (_parent->state_dependency (), _del_action);
    }
    Process::finalize ();
  }

  TextAccumulator::~TextAccumulator () {
    
    if (_parent && _parent->state_dependency () != nullptr) {
      Graph::instance ().remove_edge (_parent->state_dependency (), _acc_action);
      Graph::instance ().remove_edge (_parent->state_dependency (), _del_action);
    }
    Graph::instance ().remove_edge (_input, _acc_action);
    Graph::instance ().remove_edge (_del, _del_action);
    Graph::instance ().remove_edge (_acc_action, _state);
    Graph::instance ().remove_edge (_del_action, _state);
  
    set_vertex (nullptr);
    if (_c_acc) { delete _c_acc; _c_acc=nullptr;}
    if (_c_del) { delete _c_del; _c_del=nullptr;}
    if (_acc_action) { delete _acc_action; _acc_action=nullptr;}
    if (_del_action) { delete _del_action; _del_action=nullptr;}
    if (_del) { delete _del; _del=nullptr;}
    if (_state) { delete _state; _state=nullptr;}
    if (_input) { delete _input; _input=nullptr;}
  }

  void
  TextAccumulator::activate ()
  {
    _c_acc->enable ();
    _c_del->enable ();
  }

  void
  TextAccumulator::deactivate ()
  {
    _c_acc->disable ();
    _c_del->disable ();
  }

  void
  TextAccumulator::serialize (const string &type)
  {
    AbstractSerializer::pre_serialize (this, type);

    AbstractSerializer::serializer->start ("base:text-accumulator");
    AbstractSerializer::serializer->text_attribute ("id", _name);
    AbstractSerializer::serializer->text_attribute ("initial", _state->get_value ());

    AbstractSerializer::serializer->end ();

    AbstractSerializer::post_serialize (this);
  }
}

