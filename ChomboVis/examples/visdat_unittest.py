import chombovis
import visualizable_dataset
c=chombovis.this()

vp = c.vtk_data.reader.GetVisualizableDatasetPtr()
visdat = visualizable_dataset.VisualizableDataset( visdat_ptr=vp )
visdat.unitTest()
