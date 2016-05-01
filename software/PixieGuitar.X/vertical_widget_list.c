#include "vertical_widget_list.h"

#include <assert.h>

#include "display.h"

static bool VerticalWidgetListOnTick(Widget * instance,
                                     GfxRect const * region,
                                     int8_t knob_turn_delta,
                                     int8_t knob_press_delta,
                                     bool force_draw,
                                     WidgetState state) {
  assert(instance);
  VerticalWidgetList * me = (VerticalWidgetList *) instance;

  if (!me->_child_active) {
    if (knob_turn_delta && me->_children_count) {
      me->_highlighted_child = (me->_highlighted_child + (size_t) knob_turn_delta) %
                               me->_children_count;
    }
    knob_turn_delta = 0;

    if (knob_press_delta > 0) {
      me->_child_active = true;
      knob_press_delta = 0;
    }
  }

  int y = 0;
  for (size_t i = 0; i < me->_children_count; ++i) {
    Widget * child = me->_children[i];
    GfxRect child_region;
    if (!GfxSubRegion(region,
                      0,
                      y,
                      me->widget.width,
                      child->height,
                      &child_region)) break;

    bool selected = i == me->_highlighted_child;
    bool active = selected && me->_child_active;
    bool child_done = child->OnTick(child,
                                    &child_region,
                                    active ? knob_turn_delta : 0,
                                    active ? knob_press_delta : 0,
                                    force_draw,
                                    active   ? WIDGET_ACTIVE      :
                                    selected ? WIDGET_HIGHLIGHTED :
                                               WIDGET_INACTIVE);
    if (active && child_done) {
      me->_child_active = false;
    }

    y += child->height;
  }
  if (force_draw) {
    // Fill the remainder of the screen black.
    if (y < region->h) {
      GfxFillRect(region, 0, y, region->w, region->h - y, RGB(0, 0, 0));
    }
  }
  return false;
}

void VerticalWidgetListInit(VerticalWidgetList * instance,
                            Widget * const * children,
                            size_t children_count) {
  assert(instance);
  assert(children || !children_count);

  // Initialize super.
  instance->widget.OnTick = VerticalWidgetListOnTick;
  instance->widget.height = 0;
  instance->widget.width = 0;
  for (size_t i = 0; i < children_count; ++i) {
    Widget * child = children[i];
    assert(child);
    instance->widget.height += child->height;
    if (child->width > instance->widget.width) {
      instance->widget.width = child->width;
    }
  }

  instance->_children = children;
  instance->_children_count = children_count;
  instance->_child_active = false;
  instance->_highlighted_child = 0;
}
