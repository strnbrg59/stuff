import chombovis
c=chombovis.this()
c.slice.toggleVisibility(0)
c.cmap.setBackgroundColor( (1,1,1) )
c.cmap.hideLegend()
c.grid.setDetail( 'All cells' )
c.grid.setColor((0,0,0))
c.particles.setMarkerType( 'spheres' )
c.particles.setOpacity(0.3)
c.vtk_particles.setOffsetFilteringComponent('discrim')
c.vtk_particles.getOffsetFilterRanges()[0] = 1.5
c.vtk_particles.getOffsetFilterRanges()[1] = 2.0
c.particles.setGlyphScaleAndColorMode('color by component value')
c.misc.guiUpdate()
