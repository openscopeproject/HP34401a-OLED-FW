import os

Import("env")

srcfile = os.path.join(env.subst('$BUILD_DIR'), 'firmware.bin')
dstfile = os.path.join(env.subst('$PROJECTBUILD_DIR'),
                       'firmware_%s.bin' % env['PIOENV'])

# create a target action for copying output binary
copy_action = env.Alias('copy', 'buildprog', Copy(dstfile, srcfile))
# make scons always think this target is out of date
env.AlwaysBuild(copy_action)
# add target to default list
Default(copy_action)
