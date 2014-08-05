CHMOD = chmod
CXXFLAGS += -O2 -Wall

.PHONY: clean

%.ii: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -E $^

display-float: display-float.o analyze-float.o
	$(CXX) $(CXXFLAGS) -o $@ $^
	$(CHMOD) o+rx $@

test-analyze-float: test-analyze-float.cpp analyze-float.cpp analyze-float.h
	$(CXX) $(CXXFLAGS) -m64 -o $@ $< -lboost_unit_test_framework

display-float.o: analyze-float.h
analyze-float.o: analyze-float.h

clean:
	$(RM) display-float.o analyze-float.o test-analyze-float.o display-float test-analyze-float

test: test-analyze-float
	./test-analyze-float
