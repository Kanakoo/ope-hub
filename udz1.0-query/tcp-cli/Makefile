all:client
client: ./*.h ./*.cpp ../common/*.h ../common/*.cpp ../common/trans/*.h ../common/trans/*.cpp
	g++ ./*.h ./*.cpp ../common/*.h ../common/*.cpp ../common/trans/*.h ../common/trans/*.cpp -lpthread  -lprotobuf -std=c++11 -std=c++1y
clean:
	rm -f a.out

