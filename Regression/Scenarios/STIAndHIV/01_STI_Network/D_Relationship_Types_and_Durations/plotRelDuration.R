# SUMMARY: Plot theoretical and observed relationship duration distributions
# INPUT: 
#   1. pfa_simple.json
#   2. output/RelationshipEnd.csv
# OUTPUT: figs/RelationshipDuration.png

rm( list=ls( all=TRUE ) )
graphics.off()

library(reshape)
library(ggplot2)
library(jsonlite)

DAYS_PER_YEAR = 365

rel_names <- c('Transitory', 'Informal', 'Marital')
fig_dir = 'figs'
if( !file.exists(fig_dir) ) {
    dir.create(fig_dir)
}

end <- read.csv("output/RelationshipEnd.csv", header=TRUE)
names(end)[startsWith(names(end), 'Rel_type')] <- 'Rel_type'
end$Duration = (end$Rel_actual_end_time - end$Rel_start_time)/DAYS_PER_YEAR
end$Count = 1

C = fromJSON('../../../../STI/SFTs/Inputs/pfa_simple.json')

transitory.shape = 1.0 / C$Defaults$Society$TRANSITORY$Relationship_Parameters$Duration_Weibull_Heterogeneity
informal.shape   = 1.0 / C$Defaults$Society$INFORMAL$Relationship_Parameters$Duration_Weibull_Heterogeneity
marital.shape    = 1.0 / C$Defaults$Society$MARITAL$Relationship_Parameters$Duration_Weibull_Heterogeneity

transitory.scale = C$Defaults$Society$TRANSITORY$Relationship_Parameters$Duration_Weibull_Scale
informal.scale   = C$Defaults$Society$INFORMAL$Relationship_Parameters$Duration_Weibull_Scale
marital.scale    = C$Defaults$Society$MARITAL$Relationship_Parameters$Duration_Weibull_Scale

end$Rel_type = factor(end$Rel_type)
end.m = melt(end, id=c('Rel_type', 'Duration'), measure='Count')

p <- ggplot(data=end.m, mapping=aes(x=Duration, group=Rel_type, fill=Rel_type )) + 
    geom_density(alpha = 0.5) +
    stat_function(fun=function(x) dweibull(x, transitory.shape, transitory.scale), colour="red", linetype="dashed") +
    stat_function(fun=function(x) dweibull(x, informal.shape, informal.scale), colour="green", linetype="dashed") +
    stat_function(fun=function(x) dweibull(x, marital.shape, marital.scale), colour="blue", linetype="dashed") +
    xlab( "Duration (years)" ) +
    ylab( "Density" ) +
    scale_y_continuous(limits=c(0.0,0.8)) +
    ggtitle( "Relationship Duration by Type" ) + 
    scale_fill_discrete(name="Type", breaks=c(0,1,2), labels=rel_names)

png( file.path(fig_dir,'RelationshipDuration.png'), width=600, height=400)
print( p )
dev.off()

print(p)

