platforms=win mac lin
container=xb2c-cross-compile-env

# all: mac lin win
# all: mac lin win
all: lin win mac 

.image: Dockerfile build.sh
	docker build . -t $(container)
	docker inspect -f "{{ .ID}}" $(container):latest > .image

bash: .image
	docker run -it --rm -v "$(realpath ../src):/src" --entrypoint bash $(container):latest

$(platforms): .image
	docker run -i --rm -v "$(realpath ../src):/src" $(container):latest $@

.PHONY: $(platforms) bash
