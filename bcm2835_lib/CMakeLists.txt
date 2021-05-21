project(CFE_BCM2835_LIB C)

# Create the app module
add_cfe_app(bcm2835_lib fsw/src/bcm2835_lib.c)

# The API to this library (which may be invoked/referenced from other apps)
# is stored in fsw/public_inc.  Using "target_include_directories" is the 
# preferred method of indicating this (vs. directory-scope "include_directories").
target_include_directories(bcm2835_lib PUBLIC fsw/public_inc)



