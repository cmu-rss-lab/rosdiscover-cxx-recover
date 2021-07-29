FROM therobotcooperative/turtlebot3

#COPY --from=christimperley/llvm11 /opt/llvm11/ /opt/llvm11/
#ENV PATH "/opt/llvm11/bin:${PATH}"
#ENV LIBRARY_PATH "/opt/llvm11/lib:${LIBRARY_PATH}"
#ENV LD_LIBRARY_PATH "/opt/llvm11/lib:${LD_LIBRARY_PATH}"

#RUN apt-get update \
# && apt-get install -y file \
# && apt-get clean \
# && rm -rf /var/lib/apt/lists/*

# fix ambiguous isnan calls
# https://stackoverflow.com/questions/33770374/why-is-isnan-ambiguous-and-how-to-avoid-it
# RUN sed -i "s#isnan#std::isnan#g" /ros_ws/src/turtlebot3_simulations/turtlebot3_fake/src/turtlebot3_fake.cpp

# rebuild and generate compile_commands.json
RUN . /opt/ros/${ROS_DISTRO}/setup.sh \
 && rm -rf build \
 && catkin_make install \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=1

# install rosdiscover
#COPY --from=rosdiscover /opt/rosdiscover/ /opt/rosdiscover/
#ENV LD_LIBRARY_PATH "/opt/rosdiscover/lib:${LD_LIBRARY_PATH}"
#ENV LIBRARY_PATH "/opt/rosdiscover/lib:${LIBRARY_PATH}"
#ENV PATH "/opt/rosdiscover/bin:${PATH}"
