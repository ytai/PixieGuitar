#ifndef VERTICAL_WIDGET_LIST_H
#define	VERTICAL_WIDGET_LIST_H

#include <stddef.h>

#include "widget.h"

typedef struct {
  // "super-class".
  Widget widget;

  // private:
  Widget * const * _children;
  size_t _children_count;
  bool _child_active;
  size_t _highlighted_child;
} VerticalWidgetList;

Widget * VerticalWidgetListInit(VerticalWidgetList * instance,
                                Widget * const * children,
                                size_t children_count);

#endif  // VERTICAL_WIDGET_LIST_H

