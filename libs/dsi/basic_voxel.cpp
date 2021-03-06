#include "basic_voxel.hpp"
#include "image_model.hpp"


void Voxel::init(void)
{
    voxel_data.resize(thread_count);
    for (unsigned int index = 0; index < thread_count; ++index)
    {
        voxel_data[index].space.resize(bvalues.size());
        voxel_data[index].odf.resize(ti.half_vertices_count);
        voxel_data[index].fa.resize(max_fiber_number);
        voxel_data[index].dir_index.resize(max_fiber_number);
        voxel_data[index].dir.resize(max_fiber_number);
    }
    for (unsigned int index = 0; index < process_list.size(); ++index)
        process_list[index]->init(*this);
}


void Voxel::load_from_src(ImageModel& image_model)
{
    std::vector<int> sorted_index(image_model.src_bvalues.size());
    for(int i = 0;i < sorted_index.size();++i)
        sorted_index[i] = i;

    std::sort(sorted_index.begin(),sorted_index.end(),
              [&image_model](int left,int right)
    {
        return image_model.src_bvalues[left] < image_model.src_bvalues[right];
    }
    );


    bvalues.clear();
    bvectors.clear();
    dwi_data.clear();
    // include only the first b0
    if(image_model.src_bvalues[sorted_index[0]] == 0.0f)
    {
        bvalues.push_back(0);
        bvectors.push_back(image::vector<3,float>(0,0,0));
        dwi_data.push_back(image_model.src_dwi_data[sorted_index[0]]);
    }
    for(int i = 0;i < sorted_index.size();++i)
        if(image_model.src_bvalues[sorted_index[i]] != 0.0f)
        {
            bvalues.push_back(image_model.src_bvalues[sorted_index[i]]);
            bvectors.push_back(image_model.src_bvectors[sorted_index[i]]);
            dwi_data.push_back(image_model.src_dwi_data[sorted_index[i]]);
        }
}


void Voxel::calculate_mask(const image::basic_image<float,3>& dwi_sum)
{
    image::threshold(dwi_sum,mask,0.2f,1,0);
    if(dwi_sum.depth() < 10)
    {
        for(unsigned int i = 0;i < mask.depth();++i)
        {
            image::pointer_image<unsigned char,2> I(&mask[0]+i*mask.plane_size(),
                    image::geometry<2>(mask.width(),mask.height()));
            image::morphology::defragment(I);
            image::morphology::recursive_smoothing(I,10);
            image::morphology::defragment(I);
        }
    }
    else
    {
        image::morphology::recursive_smoothing(mask,10);
        image::morphology::defragment(mask);
        image::morphology::recursive_smoothing(mask,10);
    }
}


void Voxel::run(void)
{
    try{

    size_t total_voxel = 0;
    bool terminated = false;
    begin_prog("reconstructing");
    for(size_t index = 0;index < mask.size();++index)
        if (mask[index])
            ++total_voxel;

    unsigned int total = 0;
    image::par_for2(mask.size(),
                    [&](int voxel_index,int thread_index)
    {
        ++total;
        if(terminated || !mask[voxel_index])
            return;
        if(thread_index == 0)
        {
            if(prog_aborted())
            {
                terminated = true;
                return;
            }
            check_prog(total,mask.size());
        }
        voxel_data[thread_index].init();
        voxel_data[thread_index].voxel_index = voxel_index;
        for (int index = 0; index < process_list.size(); ++index)
            process_list[index]->run(*this,voxel_data[thread_index]);
    },thread_count);
    check_prog(1,1);
    }
    catch(std::exception& error)
    {
        std::cout << error.what() << std::endl;
    }
    catch(...)
    {
        std::cout << "unknown error" << std::endl;
    }

}


void Voxel::end(gz_mat_write& writer)
{
    begin_prog("output data");
    for (unsigned int index = 0; check_prog(index,process_list.size()); ++index)
        process_list[index]->end(*this,writer);
}

BaseProcess* Voxel::get(unsigned int index)
{
    return process_list[index].get();
}
