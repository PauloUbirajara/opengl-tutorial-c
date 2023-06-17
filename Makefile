# INPUT_FILE="00_test.c"
INPUT_FILE="01_test.c"
OUTPUT_FILE="test.exe"

build:
	g++ ${INPUT_FILE} -o ${OUTPUT_FILE} \
		-lopengl32 \
		-lglew32 \
		-lfreeglut \
		-lglu32

clean:
	del ${OUTPUT_FILE}
