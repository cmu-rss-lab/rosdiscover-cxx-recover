FROM therobotcooperative/turtlebot3

COPY --from=christimperley/llvm11 /opt/llvm11/ /opt/llvm11/
ENV PATH "/opt/llvm11/bin:${PATH}"
ENV LIBRARY_PATH "/opt/llvm11/lib:${LIBRARY_PATH}"
ENV LD_LIBRARY_PATH "/opt/llvm11/lib:${LD_LIBRARY_PATH}"

# TODO add rosdiscover bins + libs to environment
COPY --from=rosdiscover /opt/rosdiscover/ /opt/rosdiscover/

# TODO provide a portable installation of wllvm
RUN pip install setuptools==42.0.2 \
 && pip install wllvm==1.2.8

RUN apt-get update \
 && apt-get install -y file \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

# fix ambiguous isnan calls
# https://stackoverflow.com/questions/33770374/why-is-isnan-ambiguous-and-how-to-avoid-it
RUN sed -i "s#isnan#std::isnan#g" /ros_ws/src/turtlebot3_simulations/turtlebot3_fake/src/turtlebot3_fake.cpp

# rebuild via catkin_make + wllvm
ENV LLVM_COMPILER clang
ENV LLVM_COMPILER_PATH /opt/llvm11/bin
RUN . /opt/ros/${ROS_DISTRO}/setup.sh \
 && rm -rf build \
 && CFLAGS="-g -O0" CXXFLAGS="-g -O0" catkin_make install \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
      -DCMAKE_CXX_COMPILER=wllvm++ \
      -DCMAKE_COMPILER=wllvm
