#ifndef DWI_HEADER_HPP
#define DWI_HEADER_HPP
#include <vector>
#include <string>
#include "image/image.hpp"


class DwiHeader
{
	typedef std::vector<short>::iterator image_iterator;
public:
    std::string file_name, report;
    image::basic_image<unsigned short, 3> image;
	float te;
public:// for HCP dataset
    image::basic_image<float, 4> grad_dev;
    image::basic_image<unsigned char, 3> mask;
public:
    image::vector<3, float> bvec;
    float bvalue;
    float voxel_size[3];
public:
    DwiHeader(void): bvalue(0.0), te(0.0) {}
    bool open(const char* filename);
public:
    const unsigned short* begin(void) const
    {
        return &*image.begin();
    }
    unsigned short* begin(void)
    {
        return &*image.begin();
    }
    unsigned short operator[](unsigned int index) const
    {
        return image[index];
    }
    unsigned short& operator[](unsigned int index)
    {
        return image[index];
    }
    size_t size(void) const
    {
        return image.size();
    }
	void swap(DwiHeader& rhs)
	{
        image.swap(rhs.image);
        std::swap(bvec, rhs.bvec);
        std::swap(bvalue, rhs.bvalue);
        std::swap(te, rhs.te);
	}

public:

    const float* get_bvec(void) const
    {
        return &*bvec.begin();
    }
    float get_bvalue(void) const
    {
        return bvalue;
    }
    void set_bvec(float bx, float by, float bz)
	{
		bvec[0] = bx;
		bvec[1] = by;
		bvec[2] = bz;
	}
    void set_bvalue(float b)
    {
        bvalue = b;
    }
	bool operator<(const DwiHeader& rhs) const
	{
		if(bvalue != rhs.bvalue)
			return bvalue < rhs.bvalue;
		if(bvec[0] != rhs.bvec[0])
			return bvec[0] < rhs.bvec[0];
        if(bvec[1] != rhs.bvec[1])
			return bvec[1] < rhs.bvec[1];
        return bvec[2] < rhs.bvec[2];
	}
	bool operator==(const DwiHeader& rhs) const
	{
        return bvec[0] == rhs.bvec[0] &&
               bvec[1] == rhs.bvec[1] &&
               bvec[2] == rhs.bvec[2] &&
               bvalue == rhs.bvalue;
	}
public:
    static bool output_src(const char* file_name, std::vector<std::shared_ptr<DwiHeader> >& dwi_files, int upsampling,bool sort_btable);
    static bool has_b_table(std::vector<std::shared_ptr<DwiHeader> >& dwi_files);
};

#endif//DWI_HEADER_HPP
