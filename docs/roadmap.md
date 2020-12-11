
# Roadmap

- [x] Refactor to move all globals into a struct.
- [x] Make it compile with C++
- [x] Write some documentation in Markdown.
- [ ] Change 'include' to work with a search path `JOYPATH=./lib:/usr/joy/lib`
- [ ] Figure out the test suite.
- [ ] Break up interp.cc
    - [ ] Move math functions to a math.cc
    - [ ] Move file functions to a fileio.cc
    - [ ] Move combinator functions to comb.cc
    - [ ] Move "things that can eventually be implemented in joy" to prefix.cc
- [ ] Add a namespace construct, so [[LIBRE]] or [[MODULE]] blocks make name entries like `namespace.function`.
- [ ] Change default search path to `./lib:$HOME/.joy/lib:/usr/joy/lib`