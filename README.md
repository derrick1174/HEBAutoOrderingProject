# HEBAutoOrderingProject
A personal example project I attempted to integrate into an automatic ordering subsystem for HEB's Curbside system.

# Background 
I made this while applying for an entry-level position at HEB's Digital branch. I came up with the idea while applying and wanting to solve a problem relevant to this application.

HEB is a large grocery chain located primarily in Texas, USA. Their existing Curbside App allows users to manually order a cart of groceries and then reserve a place in a timeslot for HEB team members to bring their groceries out to them in the HEB parking lot and load the groceries for the customers.

# This Project
This project aims to add an optional improvement to the existing system: automatic ordering on a weekly, biweekly, or other timescale. Removing what would otherwise be the unending chore of re-ordering the same groceries every week for the rest of your life, this project would allow customers to setup a standard grocery cart of their own design, and simply arrive at an HEB site on an interval of their choosing to pick up their orders through the existing Curbside infrastructure. The users choose a range of automatic timeslots that are ideal, and a range of timeslots that (while not ideal) are still acceptable, blacklisting all nonacceptable hours. Once the project automatically places the user's order, they will receive notifications directly to their Curbside App that inform them of successful automatic orders and automatic orders that need attention, such as lack of acceptable timeslots. Users may then respond to the notification to confirm, reschedule, or cancel an order in these cases. Users may opt-in or opt-out at any time, but only one order can currently be automatically scheduled at a time in this example implementation.
