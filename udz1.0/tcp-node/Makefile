all:node
node: ./*.cpp ../common/*.h ../common/*.cpp ../common/trans/*.h ../common/trans/*.cpp
	g++ ./*.cpp ../common/*.h ../common/*.cpp ../common/trans/*.h ../common/trans/*.cpp -lpthread -std=c++11 -std=c++1y -lprotobuf
clean:
	rm -f a.out

