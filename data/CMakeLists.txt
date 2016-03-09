
# Install the scope ini file
install(
  FILES "com.ubuntu.developer.liu-xiao-guo.dianping_dianping.ini"
  DESTINATION ${SCOPE_INSTALL_DIR}
)

# Install the scope images
install(
  FILES
    "icon.png"
    "logo.png"
    "screenshot.png"
  DESTINATION
    "${SCOPE_INSTALL_DIR}"
)

configure_file(
  "com.ubuntu.developer.liu-xiao-guo.dianping_dianping-settings.ini"
  "${CMAKE_BINARY_DIR}/src/com.ubuntu.developer.liu-xiao-guo.dianping_dianping-settings.ini"
)

INSTALL(
  FILES "${CMAKE_BINARY_DIR}/src/com.ubuntu.developer.liu-xiao-guo.dianping_dianping-settings.ini"
  DESTINATION "${SCOPE_INSTALL_DIR}"
)
