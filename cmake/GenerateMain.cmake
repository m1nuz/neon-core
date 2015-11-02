macro(GENERATE_MAIN TARGET STATES STARTUP)

add_custom_command(OUTPUT main.c
COMMAND python3
ARGS ${TOOLS_PATH}/main_generator.py -n ${STATES} -s ${STARTUP}
DEPENDS ../tools/main_generator.py
COMMENT "Generating source code for main.c"
)

add_custom_target(generate_main DEPENDS main.c
COMMENT "Checking if re-generation is required"
)

add_dependencies(${TARGET} generate_main)
endmacro()
