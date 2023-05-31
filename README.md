# args2
A planned successor to the args library, using a more modular design, more
modern language features, and lessons learned from args

**This is not a usable project yet.  If you want to keep an eye on it in
development, the tests are the best place to look.  I would not recommend
actually using it, as no API stability is guaranteed at this point.  This will
be considered usable and production ready (though obviously not bug-free) on the
first 1.0.0 release.**

## Design

* Customizability is not a high priority.  Prefixes and separators might be
  customizable, but it is not a major goal of the library.  Some things like
  help generation will be primarily prescribed and not heavily customizable.
* Library should be flexible and composable.  Most of the functionality should
  be composed from smaller reusable parts.
* Tests should be extensive.  Unit tests will be automatically run across as
  many compilers as possible.
* No preprocessor directives other than `#include` and `#pragma once`.
* Code readabilitiy and maintainability is a must.

