# Connection Oriented I/O Driver Template for Xentara
This repository contains skeleton code for a Xentara driver with the following features:

Connection oriented | Batch processing | Polymorphic I/O Points
:-----------------: | :--------------: | :--------------------:
YES                 | NO               | NO

This driver template requires the Xentara development environment, as well as a Xentara licence. You can get a Xentara
licence in the [Xentara Online Shop](https://www.xentara.io/product/xentara-for-industrial-automation/).

The documentation for Xentara can be found at https://docs.xentara.io/xentara

This driver template uses the Xentara Utility Library, as well as the Xentara Plugin Framework. Docs can be found here:

https://docs.xentara.io/xentara-utils/  
https://docs.xentara.io/xentara-plugin/

## Xentara I/O Component Template

*(See [I/O Components](https://docs.xentara.io/xentara/xentara_io_components.html) in the [Xentara documentation](https://docs.xentara.io/xentara/))*

[src/TemplateIoComponent.hpp](src/TemplateIoComponent.hpp)  
[src/TemplateIoComponent.cpp](src/TemplateIoComponent.cpp)

The I/O component template provides template code for devices that may be connected and disconnected while Xentara is running.
For such I/O components, Xentara will periodically try to reestablish communication to the physical device if it is disconnected.

The template code has the following features:

- The connection to the physical device is established during the [pre-operational stage](https://docs.xentara.io/xentara/xentara_operational_stages.html#xentara_operational_stages_pre_operational),
  and closed during the [post-operational stage](https://docs.xentara.io/xentara/xentara_operational_stages.html#xentara_operational_stages_post_operational).
- The [quality](https://docs.xentara.io/xentara/xentara_io_points.html#xentara_io_points_quality) of all I/O points belonging to the component
  is set to *Bad* if communication to the physical device breaks down.
- The I/O component tracks an error code for the communication with the physical device. If communication breaks down, this error code is pushed
  to the individual I/O points.
- The I/O component publishes a [Xentara task](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_tasks) called *reconnect*,
  that checks the connection to the physical device, and attempts to reconnect if the communication has broken down.
- The I/O component publishes two [Xentara events](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_events) called *connected*
  and *disconnected*, that are fired when the connection to the physical device is establed or lost.

## Xentara I/O Point Templates

*(See [I/O Points](https://docs.xentara.io/xentara/xentara_io_points.html) in the [Xentara documentation](https://docs.xentara.io/xentara/))*

### Input Template

[src/TemplateInput.hpp](src/TemplateInput.hpp)  
[src/TemplateInput.cpp](src/TemplateInput.cpp)  

The input template provides template code for a read-only I/O point whose value must be read using an individual command for each input.

The template code has the following features:

- The input publishes a [Xentara task](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_tasks) called *read*,
  which acquires the current value from the physical device using a read command.
- The input publishes [Xentara events](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_events) to signal changes
  to the value and [quality](https://docs.xentara.io/xentara/xentara_io_points.html#xentara_io_points_quality).
- If a communication breakdown is detected during a read command, the I/O component is notified, and all other I/O points are invalidated.
- No communication with the physical device is attempted if the connection is not up.

### Output Template

[src/TemplateOutput.hpp](src/TemplateOutput.hpp)  
[src/TemplateOutput.cpp](src/TemplateOutput.cpp)

The output template provides template code for a read/write I/O point whose value must be read and written using individual commands for each output.

The template code has the following features:

- The input and output values are handled entirely separately. A written output value is not reflected in the input value until
  it has been read back from the physical device using a read command. This is necessary because the device might reject or
  modify the written value.
- The value of the output is not sent to the physical device directly when it is written, but placed in a queue to be written at
  a specificly scheduled time using a [Xentara task](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_tasks).
  This allows the user to fine-tune when and how often write commands are sent, and to coordinate reads and writes.
- The output publishes a [Xentara task](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_tasks) called *read*,
  which acquires the current value from the physical device using a read command.
- The output publishes [Xentara events](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_events) to signal changes
  to the value and [quality](https://docs.xentara.io/xentara/xentara_io_points.html#xentara_io_points_quality).
- The output publishes a [Xentara task](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_tasks) called *write*,
  which checks if an output value is pending, and writes it to the physical device using a write command, if necessary.
- The output publishes [Xentara events](https://docs.xentara.io/xentara/xentara_element_members.html#xentara_events) to signal if
  a new value was written, or if a write error occurred. 
- If a communication breakdown is detected during a read or a write command, the I/O component is notified, and all other I/O points
  are invalidated.
- No communication with the physical device is attempted if the connection is not up.
