<?php

/*
 * Load files with base classes of the contained classes.
 */

/**
 * Implementation of hook_views_plugins().
 */
function apachesolr_views_views_plugins() {
  return array(
    'module' => 'apachesolr_views',
    'query' => array(
      'apachesolr_views_query' => array(
        'title' => t('Apache Solr Query'),
        'help' => t('Query that allows you to search with Apache Solr.'),
        'handler' => 'apachesolr_views_query',
        'parent' => 'views_query',
      ),
    ),
  );

}

/**
 * Implementation of hook_views_data().
 */
function apachesolr_views_views_data() {
  foreach (module_invoke_all('apachesolr_entities') as $base_table => $definition) {
    $name = $definition['name'];
    $base_field = $definition['base_field'];
    $apachesolr_base_table = 'apachesolr_' . $base_table;
    $data[$apachesolr_base_table]['table']['group'] = t('Apache Solr');

    $data[$apachesolr_base_table]['table']['base'] = array(
      'query class' => 'apachesolr_views_query',
      'title' => t('Apache Solr @name', array('@name' => $name)),
      'help' => t('Searches the site with the Apache Solr search engine for @name', array('@name' => $name)),
      'field' => $base_field,
      'entity type' => 'node',
    );

//    $data[$apachesolr_base_table]['text'] = array(
//      'title' => t('Search'),
//      'help' => t('Searches the content with Solr'),
//      'filter' => array(
//        'handler' => 'apachesolr_views_handler_filter_search',
//      ),
//    );

//    $data[$apachesolr_base_table]['nid'] = array(
//      'title' => t('Nid'),
//      'help' => t('The node ID of the node.'),
//      'field' => array(
//        'handler' => 'views_handler_field_node',
//        'click sortable' => TRUE,
//      ),
//      'sort' => array(
//        'handler' => 'apachesolr_views_handler_sort',
//      ),
//    );

    // Get the list of the fields in idex directly from Solr.
    try {
      $solr = apachesolr_get_solr();
      $solr_fields = $solr->getFields(1);
    }
    catch (Exception $e) {
      $solr_fields = array();
      watchdog('Apache Solr Views', nl2br(check_plain($e->getMessage())), NULL, WATCHDOG_ERROR);
    }

    foreach ($solr_fields as $solr_field_name => $solr_field) {

      // We do not allow to display 'sort_*' fields.
      if (strpos($solr_field_name, 'sort_') === 0) {
        continue;
      }

      $field_type = $solr_field->type;

      $field_names = apachesolr_views_field_names($solr_field_name);

      $data[$apachesolr_base_table][$solr_field_name] = array(
        'title' => t($field_names['title']),
        'help' => t($field_names['help']),
        'field' => array(
          'handler' => 'apachesolr_views_handler_field',
          'click sortable' => TRUE,
        ),
        // Information for accepting a nid as an argument
//        'argument' => array(
//          'handler' => 'views_handler_argument_node_nid',
//          'name field' => 'title', // the field to display in the summary.
//          'numeric' => TRUE,
//          'validate type' => 'nid',
//        ),
        'filter' => array(
          'handler' => 'apachesolr_views_handler_filter',
        ),
        'sort' => array(
          'handler' => 'apachesolr_views_handler_sort',
        ),
      );
    }

  }

  return $data;
}
