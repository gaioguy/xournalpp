#include "BaseInputDevice.h"

#include "control/Control.h"
#include "gui/PageView.h"
#include "gui/widgets/XournalWidget.h"
#include "gui/XournalView.h"
#include "model/Point.h"


BaseInputDevice::BaseInputDevice(GtkWidget* widget, XournalView* view)
 : AbstractInputDevice(widget, view)
{
	XOJ_INIT_TYPE(BaseInputDevice);
}

BaseInputDevice::~BaseInputDevice()
{
	XOJ_RELEASE_TYPE(BaseInputDevice);
}

/**
 * Initialize the input handling, set input events
 */
void BaseInputDevice::initWidget()
{
	XOJ_CHECK_TYPE(BaseInputDevice);

	int events = GDK_EXPOSURE_MASK;
	events |= GDK_POINTER_MOTION_MASK;
	events |= GDK_EXPOSURE_MASK;
	events |= GDK_BUTTON_MOTION_MASK;

	// See documentation: https://developer.gnome.org/gtk3/stable/chap-input-handling.html
	events |= GDK_TOUCH_MASK;
	events |= GDK_BUTTON_PRESS_MASK;
	events |= GDK_BUTTON_RELEASE_MASK;
	events |= GDK_ENTER_NOTIFY_MASK;
	events |= GDK_LEAVE_NOTIFY_MASK;
	events |= GDK_KEY_PRESS_MASK;
	events |= GDK_SCROLL_MASK;

	// NOT Working with GTK3, only with GTK2
	// Therefore listening for mouse move events
	// of the pen, and add a timeout
	//	events |= GDK_PROXIMITY_IN_MASK;
	//	events |= GDK_PROXIMITY_OUT_MASK;

	gtk_widget_set_events(widget, events);
}

/**
 * Mouse / pen moved event
 */
bool BaseInputDevice::motionEvent(XojPageView* pageView, GdkEventMotion* event)
{
	XOJ_CHECK_TYPE(BaseInputDevice);

	double x = 0;
	double y = 0;
	double pressure = Point::NO_PRESURE;
	readPositionAndPressure(event, x, y, pressure);

	GtkXournal* xournal = GTK_XOURNAL(widget);

	double pageX = x - pageView->getX() - xournal->x;
	double pageY = y - pageView->getY() - xournal->y;

	return pageView->onMotionNotifyEvent(widget, pageX, pageY, pressure, xournal->shiftDown);
}

/**
 * Read pressure and position of the pen, if a pen is active
 */
void BaseInputDevice::readPositionAndPressure(GdkEventMotion* event, double& x, double& y, double& pressure)
{
	XOJ_CHECK_TYPE(BaseInputDevice);

	x = event->x;
	y = event->y;

	bool presureSensitivity = view->getControl()->getSettings()->isPresureSensitivity();

	if (presureSensitivity)
	{
		return;
	}

	ToolHandler* h = view->getControl()->getToolHandler();

	if (h->getToolType() != TOOL_PEN)
	{
		return;
	}

	if (!getPressureMultiplier((GdkEvent*) event, pressure))
	{
		pressure = Point::NO_PRESURE;
	}
}

/**
 * Read Pressure over GTK
 */
bool BaseInputDevice::getPressureMultiplier(GdkEvent* event, double& pressure)
{
	XOJ_CHECK_TYPE(BaseInputDevice);

	GdkDevice* device = gdk_event_get_device(event);

	// This causes some memory corruption
	// If touch and pen is active at the same time
	// Here are random crashes. It cannot be reproduced
	// without touch and pen at the same time
	// Currently only on X11 reproduced.
	// In this case DirectAxisInputDevice should solve it

	gdouble* axes = event->button.axes;
	gdk_device_get_state(device,
						 gtk_widget_get_parent_window(widget),
						 axes, NULL);

	if (!gdk_device_get_axis(device, axes, GDK_AXIS_PRESSURE, &pressure))
	{
		return false;
	}

	Settings* settings = view->getControl()->getSettings();
	pressure = ((1 - pressure) * settings->getWidthMinimumMultiplier()
			+ pressure * settings->getWidthMaximumMultiplier());

	return true;
}

/**
 * Touch event
 */
bool BaseInputDevice::touchEvent(GdkEventTouch* event)
{
	XOJ_CHECK_TYPE(BaseInputDevice);


	// This handler consume some touch events
	// Not fully working, but fixes some touch
	// issues

	// Consume event
	return true;
}

