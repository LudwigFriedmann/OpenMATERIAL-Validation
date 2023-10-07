#
# Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
#

# This file is created as an example of simple bash script which applies different
# rotation transformations to the specified scene in a loop (based on the given
# angle range) and writes the resulting images according to specified options
#


# General setting ----------------------------------------------------------------
workingdir="./"	# prepended to all filenames and paths
executable="bin/pathtracer"
inputfile="../objects/teapot_aluminium.gltf"
logfile=""
nprocs=16

# Output image params ------------------------------------------------------------
outfile="scripts/rotation"
extension=".jpg"	# .png, .bmp, .tga, .jpg, .pfm
aspect=1.0
width=2000
gamma=0.5
denoise=false

# Transformation setting ---------------------------------------------------------
scale=1.0
translation=0,0,0	# X,Y,Z
centering=false

# Rendering params ---------------------------------------------------------------
samples=10
maxpath=20
background="../hdr/green_point_park_4k.hdr"

cprops=""
cbounces=10

useLights=false
lbounces=10
lattenuation=1	# 0, 1, 2

mpriority="om"	# om, pbr
fcsubject=""	# pid, gid, mid, rmp, mmp, md, sg, mn, in


# Main script --------------------------------------------------------------------
# ~~~ flags ~~~
[[ $centering	= true ]] && flag_automaticCentering="--automaticCentering" || flag_automaticCentering=""
[[ $denoise		= true ]] && flag_denoise="--denoiser" 						|| flag_denoise=""
[[ $useLights	= true ]] && flag_lights="--lights" 						|| flag_lights=""

# ~~~ text options ~~~
[[ $background	!= "" ]] && opt_hdr="--hdr "${workingdir}${background} 				|| opt_hdr=""
[[ $fcsubject	!= "" ]] && opt_falseColor="--falseColor "${fcsubject} 				|| opt_falseColor=""
[[ $cprops		!= "" ]] && opt_cameraLens="--cameraLens "${workingdir}${cprops} 	|| opt_cameraLens=""
[[ $logfile		!= "" ]] && opt_writeToFile="--writeToFile "${workingdir}${logfile} || opt_writeToFile=""

# ~~~ common options ~~~
ptoptions="--lightAttenuation "${lattenuation}" --aspect "${aspect}" --cameraBounces "${cbounces}" --lightBounces "${lbounces}" --numberOfCores "${nprocs}" "${flag_automaticCentering}" "${flag_denoise}" "${opt_falseColor}" --gamma "${gamma}" "${opt_hdr}" "${flag_lights}" "${opt_cameraLens}" --materialPriority "${mpriority}" --maxPathLength "${maxpath}" --resolution "${width}" --samples "${samples}" --scalingFactor "${scale}" --translation "${translation}" "${opt_writeToFile}


# ~~~ Z rotation ~~~
for i in `seq 170 -50 20` ; do
	${workingdir}${executable} --input ${workingdir}${inputfile} $ptoptions --output ${outfile}"_Z_"$i${extension} --eulerAngles ${i},0,0 ;
done

# ~~~ Y rotation ~~~
for i in `seq 170 -50 20` ; do
	${workingdir}${executable} --input ${workingdir}${inputfile} $ptoptions --output ${outfile}"_Y_"$i${extension} --eulerAngles 0,${i},0 ;
done

# ~~~ X rotation ~~~
for i in `seq 170 -50 20` ; do
	${workingdir}${executable} --input ${workingdir}${inputfile} $ptoptions --output ${outfile}"_X_"$i${extension} --eulerAngles 0,0,${i} ;
done

