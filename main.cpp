#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include "tipsy_file.h"

#define DEBUG_MODE


int main(int argc, char** argv)
{
    // Check arguments
    if(argc != 4)
    {
        printf("Usage: ./tipsypath /path/to/filenames.txt snapshot_name(no extension) n_frames\nExiting...\n");
        exit(0);
    }

    // Input list of tipsy filenames, number of frames to animate
    std::string filelist = std::string(argv[1]);
    std::string snapname = std::string(argv[2]);
    std::stringstream ss;
    ss << argv[3];
    int nframes; 
    ss >> nframes;

    std::cout << "Producing " << nframes << " frames scene file from input file list: " << filelist << std::endl;

    // Read list of tipsy files
    std::vector<std::string> input_files;
    std::ifstream inp(filelist);
    if(inp.is_open())
    {
        std::string current;
        while(inp >> current)
        {
            input_files.push_back(current);
        }
    }
    else
    {
        std::cout << "Couldnt not open input file " << filelist << std::endl;
        exit(0);
    }

#ifdef DEBUG_MODE
    for(unsigned i = 0; i < input_files.size(); i++)
        std::cout << input_files[i] << std::endl;
#endif

    // Open each file and read header, add snapshot index & time to arrays
    std::vector<TipsyFile>    tipsyfiles(input_files.size());
    std::vector<std::string>  snap_ids;
    std::vector<float>        snap_times;
        snapname.append(".");
    for(unsigned i = 0; i < input_files.size(); i++)
    {
        // Open file and read header
        tipsyfiles[i].open(input_files[i].c_str());
        tipsyfiles[i].read_header(input_files[i].c_str());

        // Store snapshot ID and time
        int v = input_files[i].find(snapname);
        std::string id = input_files[i].substr(v+snapname.length(), input_files[i].length());
        snap_ids.push_back(id);
        snap_times.push_back(tipsyfiles[i].h.time);
    }

    // Check its sorted, only supporting sorted snapshots so far
    for(unsigned i = 0; i < snap_times.size()-1; i++)
        if(snap_times[i] > snap_times[i+1]) 
        {
            std::cout << "Error: snapshot times not in ascending order..." << std::endl;
            exit(0);
        }


#ifdef DEBUG_MODE
    for(unsigned i = 0; i < input_files.size(); i++)
        std::cout << input_files[i]<< ": id: " << snap_ids[i] << " time: " << snap_times[i] << std::endl;
#endif

    // Create interpolation list
    float tmin, tmax;
    tmin = *(std::min_element(snap_times.begin(), snap_times.end()));
    tmax = *(std::max_element(snap_times.begin(), snap_times.end()));
    float dt = (tmax-tmin)/nframes;

    auto clteq = [&snap_times, &snap_ids](float t){
        // Return id of snapshot with closest time lower than or equal to t
        int idx;
        for(idx = 0; idx < snap_times.size()-1; idx++)
            if(snap_times[idx+1] > t) break;
        return snap_ids[idx];
    };
    auto cgt = [&snap_times, &snap_ids](float t){
        // Return id of snapshot with closest time greater than t
        int idx;
        for(idx = snap_times.size()-1; idx >0; idx--)
            if(snap_times[idx-1] <= t) break;
        return snap_ids[idx];
    };
    auto snaptime = [&snap_times, &snap_ids, &tipsyfiles](std::string& snap){
        for(unsigned i = 0; i < snap_ids.size(); i++)
        {
            if(snap_ids[i]==snap)
                return tipsyfiles[i].h.time;
        }
        std::cout << "Error in snaptime()" << std::endl;
        exit(0);
    };

    std::vector<std::string> snr1s, snr2s;
    std::vector<float> fidxs;
    std::string snr1, snr2;
    float fidx,snr1time,snr2time;
    for (float t = tmin; t <= tmax; t += dt)
    {
      snr1 = clteq(t);
      snr2 = cgt(t);
      snr1time = snaptime(snr1);
      snr2time = snaptime(snr2);
      fidx = (t-snr1time) / (snr2time - snr1time);
      snr1s.push_back(snr1);
      snr2s.push_back(snr2);
      fidxs.push_back(fidx);
    }

    //Write snapshot_base1 snapshot_base2 fidx to file
    
    std::filebuf fb;
    fb.open("output.scene", std::ios::out);
    if(!fb.is_open())
    {
        std::cout << "Error: Could not open output file for writing 'output.scene'" << std::endl;
        exit(0);      
    }
    std::ostream scene(&fb);
    scene << "snapshot_base1 snapshot_base2 fidx" << std::endl;
    for(unsigned i = 0; i < snr1s.size(); i++)
    {
       scene  << snr1s[i] << " " << snr2s[i] << " " << fidxs[i] << std::endl;
    }
    fb.close();


    return 0;
}

