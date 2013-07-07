#include <leap_motion.h>

VALUE mLeapMotion;
VALUE cController;
VALUE cListener;
VALUE cFrame;

ID on_init;
ID on_connect;
ID on_disconnect;
ID on_exit;
ID on_frame;
ID on_focus_gained;
ID on_focus_lost;

typedef VALUE (ruby_method_vararg)(...);

class RubyListener : public Leap::Listener {
  public:
    RubyListener();
    ~RubyListener();
    VALUE getRubyListener();
    void setRubyListener(VALUE l);

    virtual void onInit(const Leap::Controller& controller);
    virtual void onConnect(const Leap::Controller& controller);
    virtual void onDisconnect(const Leap::Controller& controller);
    virtual void onFrame(const Leap::Controller&);
    virtual void onFocusGained(const Leap::Controller&);
    virtual void onFocusLost(const Leap::Controller&);

  protected:
    VALUE listener;
};

RubyListener::RubyListener()
: Leap::Listener()
{
  listener = Qnil;
}

RubyListener::~RubyListener() {
  listener = Qnil;
}

void RubyListener::setRubyListener(VALUE l) { listener = l; }
VALUE RubyListener::getRubyListener() { return listener; }

void RubyListener::onInit(const Leap::Controller& controller) {
  rb_funcall(listener, on_init, 1, rb_iv_get(listener, "@controller"));
}

void RubyListener::onConnect(const Leap::Controller& controller) {
  rb_funcall(listener, on_connect, 1, rb_iv_get(listener, "@controller"));
}

void RubyListener::onDisconnect(const Leap::Controller& controller) {
  rb_funcall(listener, on_disconnect, 1, rb_iv_get(listener, "@controller"));
}

void RubyListener::onFrame(const Leap::Controller& controller) {
  rb_funcall(listener, on_frame, 1, rb_iv_get(listener, "@controller"));
}

void RubyListener::onFocusGained(const Leap::Controller& controller) {
  rb_funcall(listener, on_focus_gained, 1, rb_iv_get(listener, "@controller"));
}

void RubyListener::onFocusLost(const Leap::Controller& controller) {
  rb_funcall(listener, on_focus_lost, 1, rb_iv_get(listener, "@controller"));
}

static VALUE dealloc(void * controller)
{
  delete reinterpret_cast<Leap::Controller*>(controller);
}

static VALUE allocate(VALUE klass)
{
  Leap::Controller * controller = new Leap::Controller();
  return Data_Wrap_Struct(klass, 0, dealloc, controller);
}

static VALUE add_listener(VALUE self, VALUE _listener)
{
  Leap::Controller * controller;
  RubyListener * listener;

  Data_Get_Struct(self, Leap::Controller, controller);
  Data_Get_Struct(_listener, RubyListener, listener);

  rb_iv_set(_listener, "@controller", self);

  if (true == controller->addListener(*listener)) {
    return Qtrue;
  }

  return Qfalse;
}

static VALUE connected_p(VALUE self)
{
  Leap::Controller * controller;

  Data_Get_Struct(self, Leap::Controller, controller);

  if (true == controller->isConnected()) {
    return Qtrue;
  }

  return Qfalse;
}

static VALUE has_focus_p(VALUE self)
{
  Leap::Controller * controller;

  Data_Get_Struct(self, Leap::Controller, controller);

  if (true == controller->hasFocus()) {
    return Qtrue;
  }

  return Qfalse;
}

static VALUE policy_flags(VALUE self)
{
  Leap::Controller * controller;

  Data_Get_Struct(self, Leap::Controller, controller);

  return INT2NUM(controller->policyFlags());
}

static VALUE remove_listener(VALUE self, VALUE _listener)
{
  Leap::Controller * controller;
  RubyListener * listener;

  Data_Get_Struct(self, Leap::Controller, controller);
  Data_Get_Struct(_listener, RubyListener, listener);

  if (true == controller->removeListener(*listener)) {
    return Qtrue;
  }

  rb_iv_set(_listener, "@controller", Qnil);

  return Qfalse;
}

static VALUE frame(VALUE self)
{
  Leap::Controller * controller;
  Leap::Frame f;
  Leap::Frame * copy;

  Data_Get_Struct(self, Leap::Controller, controller);

  f = controller->frame(0);
  copy = new Leap::Frame(f);

  return Data_Wrap_Struct(cFrame, 0, 0, copy);
}

static VALUE dealloc_listener(void * listener)
{
  delete reinterpret_cast<RubyListener*>(listener);
}

static VALUE allocate_listener(VALUE klass)
{
  RubyListener * listener = new RubyListener();
  VALUE rbobj = Data_Wrap_Struct(klass, 0, dealloc_listener, listener);
  listener->setRubyListener(rbobj);
  return rbobj;
}

static VALUE valid_p(VALUE self)
{
  Leap::Frame * f;

  Data_Get_Struct(self, Leap::Frame, f);

  if (true == f->isValid()) {
    return Qtrue;
  }

  return Qfalse;
}

static VALUE frame_id(VALUE self)
{
  Leap::Frame * f;

  Data_Get_Struct(self, Leap::Frame, f);

  return INT2NUM(f->id());
}

static VALUE frame_timestamp(VALUE self)
{
  Leap::Frame * f;

  Data_Get_Struct(self, Leap::Frame, f);

  return INT2NUM(f->timestamp());
}

static VALUE frame_invalid(VALUE self)
{
  Leap::Frame * f;
  Leap::Frame * invalid;

  Data_Get_Struct(self, Leap::Frame, f);

  invalid = new Leap::Frame(f->invalid());

  return Data_Wrap_Struct(cFrame, 0, 0, invalid);
}

static VALUE frame_to_s(VALUE self)
{
  Leap::Frame * f;
  const char * string;

  Data_Get_Struct(self, Leap::Frame, f);

  string = f->toString().c_str();
  return rb_str_new2(string);
}

void Init_leap_motion()
{
  mLeapMotion = rb_define_module("LeapMotion");
  cController = rb_define_class_under(mLeapMotion, "Controller", rb_cObject);
  cListener = rb_define_class_under(mLeapMotion, "Listener", rb_cObject);
  cFrame = rb_define_class_under(mLeapMotion, "Frame", rb_cObject);

  rb_define_alloc_func(cController, allocate);
  rb_define_method(cController, "add_listener", (ruby_method_vararg *)add_listener, 1);
  rb_define_method(cController, "remove_listener", (ruby_method_vararg *)remove_listener, 1);
  rb_define_method(cController, "connected?", (ruby_method_vararg *)connected_p, 0);
  rb_define_method(cController, "has_focus?", (ruby_method_vararg *)has_focus_p, 0);
  rb_define_method(cController, "policy_flags", (ruby_method_vararg *)policy_flags, 0);
  rb_define_method(cController, "frame", (ruby_method_vararg *)frame, 0);


  rb_define_alloc_func(cListener, allocate_listener);

  rb_define_method(cFrame, "valid?", (ruby_method_vararg *)valid_p, 0);
  rb_define_method(cFrame, "id", (ruby_method_vararg *)frame_id, 0);
  rb_define_method(cFrame, "timestamp", (ruby_method_vararg *)frame_timestamp, 0);
  rb_define_method(cFrame, "invalid", (ruby_method_vararg *)frame_invalid, 0);
  rb_define_method(cFrame, "to_s", (ruby_method_vararg *)frame_to_s, 0);

  on_init = rb_intern("on_init");
  on_connect = rb_intern("on_connect");
  on_disconnect = rb_intern("on_disconnect");
  on_exit = rb_intern("on_exit");
  on_frame = rb_intern("on_frame");
  on_focus_gained = rb_intern("on_focus_gained");
  on_focus_lost = rb_intern("on_focus_lost");
}
