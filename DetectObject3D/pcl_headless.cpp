//
//    Copyright 2021 Christopher D. McMurrough
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
/***********************************************************************************************************************
* @file pcl_headless.cpp
* @brief loads a PCD file, makes some changes, and saves an output PCD file
*
* Simple example of loading and saving PCD files, can be used as a template for processing saved data
*
* @author Christopher D. McMurrough
**********************************************************************************************************************/

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/io/ply_io.h>
#include <pcl/common/time.h>

#include <pcl/sample_consensus/model_types.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/sac_model_plane.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/sample_consensus/sac_model_sphere.h>
#include <pcl/sample_consensus/sac_model_cylinder.h>
#include <pcl/sample_consensus/sac_model_cone.h>

#include <pcl/filters/extract_indices.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/filters/voxel_grid.h>

#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/kdtree/io.h>
#include <pcl/segmentation/euclidean_cluster_comparator.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/segmentation/extract_clusters.h>

#define NUM_COMMAND_ARGS 2

enum TypeModel {BOX,SPHERE};

/***********************************************************************************************************************
* @brief Opens a point cloud file
*
* Opens a point cloud file in either PCD or PLY format
*
* @param[out] cloudOut pointer to opened point cloud
* @param[in] filename path and name of input file
* @return false if an error occurred while opening file
* @author Christopher D. McMurrough
**********************************************************************************************************************/
bool openCloud(pcl::PointCloud<pcl::PointXYZRGBA>::Ptr &cloudOut, std::string fileName)
{
    // handle various file types
    std::string fileExtension = fileName.substr(fileName.find_last_of(".") + 1);
    if(fileExtension.compare("pcd") == 0)
    {
        // attempt to open the file
        if(pcl::io::loadPCDFile<pcl::PointXYZRGBA>(fileName, *cloudOut) == -1)
        {
            PCL_ERROR("error while attempting to read pcd file: %s \n", fileName.c_str());
            return false;
        }
        else
        {
            return true;
        }
    }
    else if(fileExtension.compare("ply") == 0)
    {
        // attempt to open the file
        if(pcl::io::loadPLYFile<pcl::PointXYZRGBA>(fileName, *cloudOut) == -1)
        {
            PCL_ERROR("error while attempting to read pcl file: %s \n", fileName.c_str());
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        PCL_ERROR("error while attempting to read unsupported file: %s \n", fileName.c_str());
        return false;
    }
}

/*******************************************************************************************************************//**
 * @brief Saves a point cloud to file
 *
 * Saves a given point cloud to disk in PCD format
 *
 * @param[in] cloudIn pointer to output point cloud
 * @param[in] filename path and name of output file
 * @param[in] binaryMode saves the file in binary form if true (default:false)
 * @return false if an error occured while writing file
 * @author Christopher D. McMurrough
 **********************************************************************************************************************/
bool saveCloud(const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr &cloudIn, std::string fileName, bool binaryMode=true)
{
    // if the input cloud is empty, return
    if(cloudIn->points.size() == 0)
    {
        return false;
    }

    // attempt to save the file
    if(pcl::io::savePCDFile<pcl::PointXYZRGBA>(fileName, *cloudIn, binaryMode) == -1)
    {
        PCL_ERROR("error while attempting to save pcd file: %s \n", fileName);
        return false;
    }
    else
    {
        return true;
    }
}

/*******************************************************************************************************************//**
 * @brief Locate a plane in the cloud
 *
 * Perform planar segmentation using RANSAC, returning the plane parameters and point indices
 *
 * @param[in] cloudIn pointer to input point cloud
 * @param[out] inliers list containing the point indices of inliers
 * @param[in] distanceThreshold maximum distance of a point to the planar model to be considered an inlier
 * @param[in] maxIterations maximum number of iterations to attempt before returning
 * @return the number of inliers
 * @author Christopher D. McMurrough
 **********************************************************************************************************************/
void segmentPlane(const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr &cloudIn,
    std::vector<pcl::ModelCoefficients::Ptr> &allPlanes,
    std::vector<pcl::PointIndices::Ptr> &allindices,
    double distanceThreshold, 
    int maxIterations, TypeModel type_model)
{
    // store the model coefficients
    pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients);
    pcl::PointIndices::Ptr inliers(new pcl::PointIndices);
    // Create the segmentation object for the planar model and set the parameters
    pcl::SACSegmentation<pcl::PointXYZRGBA> seg;
    seg.setOptimizeCoefficients(true);
    if(type_model == BOX)
        seg.setModelType(pcl::SACMODEL_PLANE);
    else if(type_model == SPHERE)
        seg.setModelType(pcl::SACMODEL_SPHERE );
    seg.setMethodType(pcl::SAC_RANSAC);
    seg.setMaxIterations(maxIterations);
    seg.setDistanceThreshold(distanceThreshold);

    // Segment the largest planar component from the remaining cloud
    seg.setInputCloud(cloudIn);
    seg.segment(*inliers, *coefficients);
    allPlanes.push_back(coefficients);
    allindices.push_back(inliers);
    // std::cout<<"Distance:"<<coefficients->values[3]<<std::endl;
}




/***********************************************************************************************************************
* @brief program entry point
* @param[in] argc number of command line arguments
* @param[in] argv string array of command line arguments
* @returnS return code (0 for normal termination)
* @author Christoper D. McMurrough
**********************************************************************************************************************/
int main(int argc, char** argv)
{
    // validate and parse the command line arguments
    if(argc <= 1)
    {
        std::printf("USAGE: %s <file_name>\n", argv[0]);
        return 0;
    }
	std::string inputFilePath(argv[1]);
	std::string outputFilePath(argv[2]);

    // create a stop watch for measuring time
    pcl::StopWatch watch;

    // open the point cloud
    pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
    openCloud(cloud, inputFilePath);

	// start timing the processing step
    watch.reset();

    // Statistical OutlierRemoval
    // pcl::StatisticalOutlierRemoval<pcl::PointXYZRGBA> sor;
    // sor.setInputCloud (cloud);
    // sor.setMeanK (50);
    // sor.setStddevMulThresh (1.0);
    // sor.filter (*cloud);

    // 
    std::vector<pcl::ModelCoefficients::Ptr> allPlanes;
    std::vector<pcl::PointIndices::Ptr> allindices;

    // segment a plane
    const float distanceThreshold = 0.0254; // 0.0254
    const int maxIterations = 5000;
    segmentPlane(cloud,allPlanes,allindices, distanceThreshold, maxIterations,BOX);
    //std::cout << "Segmentation result: " << inliers->indices.size() << " points" << std::endl;
    
    // color the plane inliers green
    for(int i = 0; i < allindices.at(0)->indices.size(); i++)
    {
        int index = allindices.at(0)->indices.at(i);
        cloud->points.at(index).r = 0;
        cloud->points.at(index).g = 0;
        cloud->points.at(index).b = 255;
        // std::cout<<cloud->points.at(index).z<<std::endl;
    }
    double a = allPlanes.at(0)->values[0];
    double b = allPlanes.at(0)->values[1];
    double c = allPlanes.at(0)->values[2];
    double d = allPlanes.at(0)->values[3];

   

    // for(int i=0;i<cloud->points.size();i++)
    // {
    //     std::cout<< pcl::pointToPlaneDistance(cloud->points.at(i),a,b,c,d)<<std::endl;
    //     //std::cout<< a<<std::endl;
    // }

    // filtered the planes
    pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloudFiltered(new pcl::PointCloud<pcl::PointXYZRGBA>);
    pcl::ExtractIndices<pcl::PointXYZRGBA> extract;
    extract.setInputCloud(cloud);
    extract.setIndices (allindices.at(0));
    extract.setNegative (true);
    extract.setKeepOrganized(true);
    extract.filter (*cloudFiltered);

    // // downsample the cloud using a voxel grid filter
    // const float voxelSize = 0.01;
    // pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloudFiltered(new pcl::PointCloud<pcl::PointXYZRGBA>);
    // pcl::VoxelGrid<pcl::PointXYZRGBA> voxFilter;
    // voxFilter.setInputCloud(cloudFiltered);
    // voxFilter.setLeafSize(static_cast<float>(voxelSize), static_cast<float>(voxelSize), static_cast<float>(voxelSize));
    // voxFilter.filter(*cloudFiltered);
    // std::cout << "Points before downsampling: " << cloud->points.size() << std::endl;
    // std::cout << "Points before downsampling: " << cloudFiltered->points.size() << std::endl;

    
    // create the vector of indices lists (each element contains a list of imultiple indices)
    const float clusterDistance = 0.02;
    int minClusterSize = 50;
    int maxClusterSize = 100000;
    std::vector<pcl::PointIndices> clusterIndices;

    // // Creating the KdTree object for the search method of the extraction
    pcl::search::KdTree<pcl::PointXYZRGBA>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZRGBA>);
    tree->setInputCloud(cloudFiltered);

    // create the euclidian cluster extraction object
    pcl::EuclideanClusterExtraction<pcl::PointXYZRGBA> ec;
    ec.setClusterTolerance(clusterDistance);
    ec.setMinClusterSize(minClusterSize);
    ec.setMaxClusterSize(maxClusterSize);
    ec.setSearchMethod(tree);
    ec.setInputCloud(cloudFiltered);

    // // perform the clustering
    ec.extract(clusterIndices);
    //std::cout << "Clusters identified: " << clusterIndices.size() << std::endl;

    int spherical = 0;
    int boxes = 0;
    // // color each cluster
    for(int i = 0; i < clusterIndices.size(); i++)
    {
        // create a random color for this cluster
        // int r = rand() % 256;
        // int g = rand() % 256;
        // int b = rand() % 256;

        // iterate through the cluster points
        // for(int j = 0; j < clusterIndices.at(i).indices.size(); j++)
        // {
        //     cloudFiltered->points.at(clusterIndices.at(i).indices.at(j)).r = r;
        //     cloudFiltered->points.at(clusterIndices.at(i).indices.at(j)).g = g;
        //     cloudFiltered->points.at(clusterIndices.at(i).indices.at(j)).b = b;
        // }
        double mean = 0;

        for(int j = 0; j < clusterIndices.at(i).indices.size(); j++){
            mean +=pcl::pointToPlaneDistance(cloudFiltered->points.at(clusterIndices.at(i).indices.at(j)),a,b,c,d);
        }
        mean/=clusterIndices.at(i).indices.size();
        
         // iterate through the cluster points
        if(mean<0.20)
        {
            // std::cout<< "Cluster" << i<<": "<< mean<<std::endl;
            int r=0,g=0,b=0;
            if(mean>0.07 && mean < 0.10)
            {
                r= 255;
                spherical++;
            }
            else{
                g = 255;
                boxes++;
            }
            
            for(int j = 0; j < clusterIndices.at(i).indices.size(); j++)
            {
                cloud->points.at(clusterIndices.at(i).indices.at(j)).r = r;
                cloud->points.at(clusterIndices.at(i).indices.at(j)).g = g;
                cloud->points.at(clusterIndices.at(i).indices.at(j)).b = b;
            }
        }

    }
    
    std::cout<<"Boxes Count: "<< boxes << std::endl;
    std::cout<<"Spherical Count: "<< spherical << std::endl;

    
    
    // segmentPlane(cloudFiltered,allPlanes,allindices, distanceThreshold, maxIterations,SPHERE);

    // for(int i = 0; i < allindices.at(1)->indices.size(); i++)
    // {
    //     int index = allindices.at(1)->indices.at(i);
    //     cloud->points.at(index).r = 0;
    //     cloud->points.at(index).g = 255;
    //     cloud->points.at(index).b = 0;
    //     // std::cout<<cloud->points.at(index).z<<std::endl;
    // }
    

    

	
	// // color all of the points random colors
	// for(int i = 0; i < cloud->points.size(); i++)
	// {
	// 	cloud->points.at(i).r = rand() % 256;
	// 	cloud->points.at(i).g = rand() % 256;
	// 	cloud->points.at(i).b = rand() % 256;
	// }

    // get the elapsed time
    double elapsedTime = watch.getTimeSeconds();
    std::cout << elapsedTime << " seconds passed " << std::endl;

    // save the point cloud
	saveCloud(cloud, outputFilePath);

    // exit program
    return 0;
}
